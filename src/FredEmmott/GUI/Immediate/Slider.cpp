// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/ToolTip.hpp>
#include <FredEmmott/utility/almost_equal.hpp>
#include <stdexcept>
#include <utility>

#include "FredEmmott/GUI/StaticTheme/Slider.hpp"
#include "PopupWindow.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {
namespace {

struct SliderImmediateContext : Widgets::Context {
  enum class ToolTipReason { Dragging, Hover };

  ImmutableStyle mRootStyle;
  ImmutableStyle mCenteringContainerStyle;
  SliderResultMixin::value_formatter_t mValueFormatter {nullptr};
  std::optional<ToolTipReason> mToolTipReason;

  std::string FormatValue(const Widgets::Slider* const w, const float value)
    const {
    if (mValueFormatter) {
      return mValueFormatter(value);
    }
    const auto freq = w->GetStepFrequency();
    const bool isIntegral = utility::almost_equal(freq, std::round(freq));
    if (isIntegral) {
      return std::to_string(std::llround(value));
    }
    return std::to_string(value);
  }
};

[[nodiscard]]
SliderResult
SliderImpl(float* const pValue, const Orientation orientation, const ID id) {
  if (!pValue) [[unlikely]] {
    throw std::logic_error("Slider requires a non-null value pointer");
  }

  const auto w = ChildlessWidget<Widgets::Slider>(id, orientation);

  const auto changed = std::exchange(w->mChanged, false);
  if (changed) {
    *pValue = w->GetValue();
  } else {
    w->SetValue(*pValue);
    w->mChanged = false;
  }

  const auto ctx = w->GetOrCreateContext<SliderImmediateContext>();
  if (w->ConsumeWasThumbHovered()) {
    ctx->mToolTipReason = SliderImmediateContext::ToolTipReason::Hover;
  }
  if (w->IsDragging()) {
    ctx->mToolTipReason = SliderImmediateContext::ToolTipReason::Dragging;
  } else if (
    ctx->mToolTipReason == SliderImmediateContext::ToolTipReason::Dragging) {
    ctx->mToolTipReason.reset();
  }

  if (!ctx->mToolTipReason.has_value()) {
    return {w, changed};
  }

  using namespace StaticTheme::ToolTip;
  static constexpr auto ArbitraryHeight = ToolTipContentThemeFontSize * 3;
  static constexpr auto ArbitraryPadding = ArbitraryHeight;
  const auto parentWindow = tWindow;
  const bool isHorizontal = (orientation == Orientation::Horizontal);

  if (!BeginBasicPopupWindow(ID("{}/Popup", id.GetValue())).Transparent()) {
    ctx->mToolTipReason.reset();
    return {w, changed};
  }

  if (!tWindow->GetNativeHandle().mValue) {
    const auto width = ToolTipMaxWidth
      + (isHorizontal ? YGNodeLayoutGetWidth(w->GetLayoutNode()) : 0);
    const auto height = isHorizontal
      ? std::numeric_limits<float>::quiet_NaN()
      : (YGNodeLayoutGetHeight(w->GetLayoutNode()) + ToolTipContentThemeFontSize
         + (2 * ArbitraryPadding));
    const auto windowOffset = isHorizontal
      ? Point {ToolTipMaxWidth / 2, 56}
      : Point {ToolTipMaxWidth + 12, ArbitraryPadding};
    const auto windowOrigin
      = w->GetTopLeftCanvasPoint() + w->GetTrackOriginOffset() - windowOffset;

    tWindow->SetIsToolTip();
    tWindow->SetInitialPositionInNativeCoords(
      parentWindow->CanvasPointToNativePoint(windowOrigin));
    ctx->mRootStyle = ImmutableStyle {
      Style().Height(height).Width(width).PaddingTop(
        isHorizontal ? 0 : windowOffset.mY)
      //.BackgroundColor(Color::Constant::FromRGBA32(0x550000FF))
    };
    ctx->mCenteringContainerStyle = ImmutableStyle {
      Style()
        .Height(
          isHorizontal ? std::numeric_limits<float>::quiet_NaN()
                       : ArbitraryHeight)
        .MarginTop(
          isHorizontal ? 0 : (w->GetTrackLength() - (ArbitraryHeight / 2)))
        .Width(ToolTipMaxWidth)
        .AlignItems(YGAlignFlexEnd)
        .AlignContent(YGAlignFlexEnd)
        .JustifyContent(YGJustifyCenter)
        .FlexDirection(
          isHorizontal ? YGFlexDirectionRow : YGFlexDirectionColumn)
      //.BackgroundColor(Color::Constant::FromRGBA32(0x5500FF00))
    };
  }
  // We have some surprisingly deep nesting here, so let's go!
  // 1. Root: specifies the width of the window
  //
  // This is larger than we need to that it provides a canvas that doesn't
  // need moving or resizing as the 'tooltip' moves
  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"Slider/ValueToolTip/Root"}, ctx->mRootStyle);

  const auto value
    = w->IsDragging() ? w->GetSnappedDraggingValue() : w->GetValue();
  const auto [min, max] = w->GetRange();
  const auto pixelOffset = w->IsDragging()
    ? w->GetDraggingTrackOffset()
    : (w->GetTrackLength() * (value - min) / (max - min));

  // 2. Centering container: Probably bigger than the content, just centers it
  // Because of the relationship between the two widths, this is centered over
  // the offset, which we mutate with TranslateX
  const auto inner = BeginWidget<Widget>(
    ID {0},
    LiteralStyleClass {"Slider/ValueToolTip/CenteringContainer"},
    ctx->mCenteringContainerStyle);
  if (isHorizontal) {
    inner->SetMutableStyles(Style().TranslateX(pixelOffset));
  } else {
    inner->SetMutableStyles(Style().TranslateY(-pixelOffset));
  }
  // 3. The actual tooltip: styling only, adjusts to size of label inside
  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"Slider/ValueToolTip"}, DefaultToolTipStyle());

  // 4. The text :)
  Label(ctx->FormatValue(w, value), ID {0});

  EndWidget();
  EndWidget();
  EndWidget();
  EndBasicPopupWindow();

  return {w, changed};
}
}// namespace

void SliderResultMixin::SetValueFormatter(
  Widgets::Slider* w,
  const value_formatter_t fn) {
  w->GetOrCreateContext<SliderImmediateContext>()->mValueFormatter = fn;
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail

namespace FredEmmott::GUI::Immediate {
SliderResult HSlider(float* pValue, const ID id) {
  return immediate_detail::SliderImpl(pValue, Orientation::Horizontal, id);
}

SliderResult VSlider(float* pValue, const ID id) {
  return immediate_detail::SliderImpl(pValue, Orientation::Vertical, id);
}

}// namespace FredEmmott::GUI::Immediate
