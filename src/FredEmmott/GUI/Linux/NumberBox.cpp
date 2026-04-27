// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux NumberBox: minimum viable two-way binding between the underlying
// TextBox and a float* pValue. Real value parsing/formatting (locale-aware,
// range-clamped, formatter callbacks) lives in the cross-platform
// Immediate/NumberBox.cpp which is currently wchar_t/UChar-coupled and not
// yet portable. For now we use plain std::to_string / std::stof; the demo's
// float input round-trips and the user can edit the value as text.

#include <FredEmmott/GUI/Immediate/NumberBox.hpp>

#include <charconv>
#include <cmath>
#include <cstdio>
#include <string>
#include <unordered_map>

#include <FredEmmott/GUI/FocusManager.hpp>
#include <FredEmmott/GUI/Immediate/Result.hpp>
#include <FredEmmott/GUI/Widgets/TextBox.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {

void NumberBoxResultMixin::SetRange(Widgets::Widget*, float, float) {
}
void NumberBoxResultMixin::SetSmallStep(Widgets::Widget*, float) {
}
void NumberBoxResultMixin::SetValueFormatter(
  Widgets::Widget*,
  value_formatter_t) {
}
void NumberBoxResultMixin::SetValueFilter(Widgets::Widget*, value_filter_t) {
}

}// namespace immediate_detail

namespace {

std::string FormatFloat(float v) {
  // Cross-platform NumberBox<optional<T>> uses NaN as the sentinel for
  // "no value". Display that as an empty TextBox rather than literal
  // "nan" so the user can type a real number without first deleting.
  if (std::isnan(v)) {
    return {};
  }
  // Trim trailing zeros from a "%g"-style format so 1.5 stays "1.5"
  // (not "1.500000") and integral values stay free of decimal noise.
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%g", static_cast<double>(v));
  return buf;
}

bool ValuesEqualOrBothNaN(float a, float b) {
  if (std::isnan(a) && std::isnan(b)) {
    return true;
  }
  return a == b;
}

bool ParseFloat(std::string_view text, float& out) {
  const char* begin = text.data();
  const char* end = begin + text.size();
  // std::from_chars on float is C++17 and avoids locale dependence (unlike
  // std::stof which would convert in the current locale and bite us on
  // non-en_US systems).
  float value = 0.0f;
  const auto [ptr, ec] = std::from_chars(begin, end, value);
  if (ec != std::errc {} || ptr != end) {
    return false;
  }
  out = value;
  return true;
}

}// namespace

NumberBoxResult NumberBox(float* pValue, const ID id) {
  const auto w = immediate_detail::BeginWidget<Widgets::TextBox>(id);
  immediate_detail::EndWidget<Widgets::TextBox>();

  bool changed = false;

  if (pValue) {
    // Track the *pValue we last reflected into the TextBox per widget.
    // Without this, a single user edit triggers a ping-pong: frame N
    // ConsumeWasChanged is true (we parse), frame N+1 it's false (we
    // reformat *pValue and overwrite the user's still-incomplete text).
    // The map is keyed by the widget pointer, which is stable across
    // frames in immediate mode. Entries leak when widgets are destroyed;
    // good enough for now, real lifetime is a follow-up.
    static thread_local std::unordered_map<Widgets::Widget*, float> sLastSeen;

    const bool userEdited = w->ConsumeWasChanged();
    auto [it, inserted] = sLastSeen.try_emplace(w, *pValue);
    const bool externalUpdate
      = inserted || !ValuesEqualOrBothNaN(it->second, *pValue);

    if (userEdited) {
      float parsed = 0.0f;
      if (ParseFloat(w->GetText(), parsed)) {
        if (!ValuesEqualOrBothNaN(parsed, *pValue)) {
          *pValue = parsed;
          changed = true;
        }
      } else if (w->GetText().empty()) {
        // Empty box = "no value" → NaN, matches NumberBox<optional>'s
        // sentinel so optional values can be cleared by deleting.
        const float nan = std::numeric_limits<float>::quiet_NaN();
        if (!ValuesEqualOrBothNaN(nan, *pValue)) {
          *pValue = nan;
          changed = true;
        }
      }
      // Invalid non-empty text (mid-edit "1.", "abc"): leave *pValue and
      // the visible text alone so the user can keep typing.
      it->second = *pValue;
    } else if (externalUpdate) {
      // Don't reseed while the box is focused. The cross-platform
      // NumberBox<optional<int>> wrapper rounds the user's typed float
      // (e.g. "1.5" → 2) and writes back; that round-trip is detected
      // here as an external update and would otherwise stomp the
      // in-progress edit ("1.5" overwritten with "2" on the very next
      // frame). Defer reseed until the user clicks out of the box,
      // which is the natural commit point.
      bool isFocused = false;
      if (auto* const window = w->GetOwnerWindow()) {
        if (auto* const fm = window->GetFocusManager()) {
          isFocused = fm->IsWidgetFocused(w);
        }
      }
      if (!isFocused) {
        w->SetText(FormatFloat(*pValue));
        // SetText flips mWasChanged; consume it now so next frame we
        // don't take the userEdited branch.
        std::ignore = w->ConsumeWasChanged();
        it->second = *pValue;
      }
    }
  }

  return {w, changed};
}

}// namespace FredEmmott::GUI::Immediate
