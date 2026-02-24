// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "NumberBox.hpp"

#include <felly/unique_ptr.hpp>

#include "Disabled.hpp"
#include "FredEmmott/GUI/StaticTheme/TextBox.hpp"
#include "FredEmmott/GUI/Widgets/Button.hpp"
#include "FredEmmott/GUI/Widgets/TextBox.hpp"
#include "FredEmmott/GUI/detail/icu.hpp"
#include "FredEmmott/utility/almost_equal.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {
using detail::uchar_cast;
using unique_unum = felly::unique_ptr<UNumberFormat, &unum_close>;

struct NumberBoxContext : Widgets::Context {
  ~NumberBoxContext() override = default;

  float mValue {std::numeric_limits<float>::quiet_NaN()};
  float mMinimum {std::numeric_limits<float>::quiet_NaN()};
  float mMaximum {std::numeric_limits<float>::quiet_NaN()};
  float mSmallStep {1.0f};
  bool mIsIntegral {true};

  std::string mText {};
  unique_unum mIcu {nullptr};

  immediate_detail::NumberBoxResultMixin::value_formatter_t mValueFormatter {
    nullptr};
  immediate_detail::NumberBoxResultMixin::value_filter_t mValueFilter {nullptr};

  [[nodiscard]]
  float FilterValue(const float value) const {
    if (mValueFilter) {
      return mValueFilter(mValue, value);
    }
    if (std::isnan(value)) {
      return value;
    }
    if ((!std::isnan(mMinimum)) && value < mMinimum) {
      return mMinimum;
    }
    if ((!std::isnan(mMaximum)) && value > mMaximum) {
      return mMaximum;
    }
    return value;
  }

  [[nodiscard]]
  std::variant<std::monostate, std::string, std::wstring_view> FormatValue(
    const float value) {
    if (mValueFormatter) {
      return mValueFormatter(value);
    }
    if (std::isnan(value)) {
      return std::wstring_view {};
    }
    thread_local std::wstring buffer;
    buffer.resize(64);
    UErrorCode status = U_ZERO_ERROR;
    int32_t count = -1;
    if (mIsIntegral) {
      const auto rounded = std::llround(value);
      count = unum_formatInt64(
        GetICU(),
        rounded,
        uchar_cast(buffer.data()),
        felly::numeric_cast<int32_t>(buffer.size()),
        nullptr,
        &status);
    } else {
      count = unum_formatDouble(
        GetICU(),
        value,
        uchar_cast(buffer.data()),
        felly::numeric_cast<int32_t>(buffer.size()),
        nullptr,
        &status);
    }
    if (U_FAILURE(status)) {
      return {};
    }
    const auto idx = buffer.find_last_not_of(L'\0', count - 1);
    if (idx != std::wstring::npos) {
      return std::wstring_view {buffer.data(), idx + 1};
    }
    return std::wstring_view {};
  }

  [[nodiscard]]
  UNumberFormat* GetICU() {
    if (!mIcu) {
      UErrorCode status = U_ZERO_ERROR;
      mIcu.reset(unum_open(UNUM_DEFAULT, nullptr, 0, "", nullptr, &status));
      FUI_ASSERT(U_SUCCESS(status));
    }
    return mIcu.get();
  }

  // - nullopt: parse failure
  // - NaN: no value
  // - valid: parsed
  [[nodiscard]]
  std::optional<float> ParseValue(const std::wstring_view text) {
    if (text.empty()) {
      return std::numeric_limits<float>::quiet_NaN();
    }

    UErrorCode status = U_ZERO_ERROR;
    const auto parsed = unum_parse(
      GetICU(), uchar_cast(text.data()), text.size(), nullptr, &status);
    if (U_FAILURE(status)) {
      return std::nullopt;
    }
    return FilterValue(felly::numeric_cast<float>(parsed));
  }

  [[nodiscard]]
  bool UpdateText(Widgets::TextBox* w, const float value) {
    return std::visit(
      felly::overload {
        [](std::monostate) { return false; },
        [w, this](const std::string& s) {
          w->SetText(this->mText = s);
          if (w->ConsumeWasChanged()) {
            w->SelectAll();
          }
          return true;
        },
        [w, this](const std::wstring_view s) {
          w->SetTextW(s);
          this->mText = w->GetText();
          if (w->ConsumeWasChanged()) {
            w->SelectAll();
          }
          return true;
        }},
      this->FormatValue(value));
  }
};

}// namespace

namespace immediate_detail {
void NumberBoxResultMixin::SetValueFormatter(
  Widgets::Widget* w,
  value_formatter_t f) {
  const auto ctx = w->GetOrCreateContext<NumberBoxContext>();
  if (ctx->mValueFormatter == f)
    return;
  ctx->mValueFormatter = f;
  const auto tb = static_cast<Widgets::TextBox*>(w);
  if (!ctx->UpdateText(tb, ctx->mValue)) {
    ctx->mValue = std::numeric_limits<float>::quiet_NaN();
    ctx->mText.clear();
  }
}

void NumberBoxResultMixin::SetValueFilter(
  Widgets::Widget* w,
  value_filter_t f) {
  const auto ctx = w->GetOrCreateContext<NumberBoxContext>();
  if (ctx->mValueFilter == f)
    return;
  const auto newValue = ctx->FilterValue(ctx->mValue);
  if (ctx->UpdateText(static_cast<Widgets::TextBox*>(w), newValue)) {
    ctx->mValue = newValue;
  }
}

void NumberBoxResultMixin::SetRange(
  Widgets::Widget* w,
  const float min,
  const float max) {
  const auto ctx = w->GetOrCreateContext<NumberBoxContext>();
  if (ctx->mMinimum == min && ctx->mMaximum == max)
    return;
  ctx->mMinimum = min;
  ctx->mMaximum = max;
  const auto clamped = std::clamp(ctx->mValue, ctx->mMinimum, ctx->mMaximum);
  if (ctx->UpdateText(static_cast<Widgets::TextBox*>(w), clamped)) {
    ctx->mValue = clamped;
  } else {
    ctx->mText.clear();
    ctx->mValue = std::numeric_limits<float>::quiet_NaN();
  }
}

void NumberBoxResultMixin::SetSmallStep(
  Widgets::Widget* w,
  const float interval) {
  const auto ctx = w->GetOrCreateContext<NumberBoxContext>();
  ctx->mSmallStep = interval;
  ctx->mIsIntegral = utility::almost_equal(interval, std::round(interval));
}
}// namespace immediate_detail

[[nodiscard]]
NumberBoxResult NumberBox(float* const value, const ID id) {
  using namespace StaticTheme::TextBox;

  if (!value) [[unlikely]] {
    throw std::logic_error("NumberBox requires a non-null value pointer");
  }

  const auto w = immediate_detail::BeginWidget<Widgets::TextBox>(id);
  const auto endWidget = felly::scope_exit([] {
    immediate_detail::EndWidget<Widgets::TextBox>();
  });

  const auto ctx = w->GetOrCreateContext<NumberBoxContext>();
  const auto IsUnchanged = [ctx, value] {
    return (ctx->mValue == *value)
      || (std::isnan(ctx->mValue) && std::isnan(*value));
  };
  if (!IsUnchanged()) {
    if (ctx->UpdateText(w, *value)) {
      ctx->mValue = *value;
    } else {
      *value = std::numeric_limits<float>::quiet_NaN();
      ctx->mText.clear();
    }
  }

  const auto sync = felly::scope_exit([&] {
    if (IsUnchanged()) {
      return;
    }
    if (!ctx->UpdateText(w, ctx->mValue)) {
      *value = std::numeric_limits<float>::quiet_NaN();
      ctx->mText.clear();
      return;
    }

    *value = ctx->mValue;
    if (const auto fm = FocusManager::Get()) {
      fm->GivePointerFocus(w);
    }
  });

  // Typing
  if (w->ConsumeWasChanged()) {
    const auto parsed = ctx->ParseValue(w->GetTextW());
    if (parsed) {
      ctx->mValue = *parsed;
    }
  }

  if (!w->GetText().empty()) {
    const auto clearButton = immediate_detail::BeginWidget<Widgets::Button>(
      ID {0},
      DefaultTextBoxButtonStyle(),
      StyleClasses {TextBoxButtonInvisibleWhenInactiveStyleClass});
    Label("\ue894");
    immediate_detail::EndWidget<Widgets::Button>();

    if (clearButton->ConsumeWasActivated()) {
      ctx->mValue = ctx->FilterValue(std::numeric_limits<float>::quiet_NaN());
    }
  }

  BeginDisabled(std::isnan(ctx->mValue));
  BeginEnabled(std::isnan(ctx->mMaximum) || ctx->mValue < ctx->mMaximum);
  const auto incrementButton = immediate_detail::BeginWidget<Widgets::Button>(
    ID {1}, DefaultTextBoxButtonStyle(), StyleClasses {});
  Label("\ue70e");
  immediate_detail::EndWidget<Widgets::Button>();
  EndEnabled();

  if (incrementButton->ConsumeWasActivated()) {
    ctx->mValue = ctx->FilterValue(ctx->mValue + ctx->mSmallStep);
  }

  BeginEnabled(std::isnan(ctx->mMinimum) || ctx->mValue > ctx->mMinimum);
  const auto decrementButton = immediate_detail::BeginWidget<Widgets::Button>(
    ID {2}, DefaultTextBoxButtonStyle(), StyleClasses {});
  Label("\ue70d");
  immediate_detail::EndWidget<Widgets::Button>();
  EndEnabled();
  EndDisabled();

  if (decrementButton->ConsumeWasActivated()) {
    ctx->mValue = ctx->FilterValue(ctx->mValue - ctx->mSmallStep);
  }

  return {w, (ctx->mValue != *value)};
}

}// namespace FredEmmott::GUI::Immediate