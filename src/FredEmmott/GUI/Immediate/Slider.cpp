// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/ToolTip.hpp>
#include <FredEmmott/utility/almost_equal.hpp>
#include <Yoga.h>
#include <stdexcept>
#include <utility>

#include "FredEmmott/GUI/StaticTheme/Slider.hpp"
#include "PopupWindow.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {
namespace {

struct SliderImmediateContext : Widgets::Context {
  enum class ToolTipReason {
    Dragging,
    Hover,
    KeyboardInput,
  };

  SliderResultMixin::value_formatter_t mValueFormatter {nullptr};
  std::optional<ToolTipReason> mToolTipReason;

  [[nodiscard]]
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
SliderResult SliderImpl(
  float* const pValue,
  const float minimum,
  const float maximum,
  const Orientation orientation,
  const ID id) {
  if (!pValue) [[unlikely]] {
    throw std::logic_error("Slider requires a non-null value pointer");
  }
  FUI_ASSERT(*pValue >= minimum);
  FUI_ASSERT(*pValue <= maximum);

  const auto w = ChildlessWidget<Widgets::Slider>(id, orientation);

  const auto changed = w->ConsumeWasChanged();
  if (changed) {
    *pValue = w->GetValue();
  } else {
    w->SetValue(*pValue);
    std::ignore = w->ConsumeWasChanged();
  }

  w->SetRange(minimum, maximum);

  const auto ctx = w->GetOrCreateContext<SliderImmediateContext>();
  if (w->ConsumeWasThumbStationaryHovered()) {
    ctx->mToolTipReason = SliderImmediateContext::ToolTipReason::Hover;
  }
  if (w->ConsumeDidReceiveKeyboardInput()) {
    ctx->mToolTipReason = SliderImmediateContext::ToolTipReason::KeyboardInput;
  }
  if (w->IsDragging()) {
    ctx->mToolTipReason = SliderImmediateContext::ToolTipReason::Dragging;
  } else if (
    ctx->mToolTipReason == SliderImmediateContext::ToolTipReason::Dragging) {
    ctx->mToolTipReason.reset();
  }
  if (
    ctx->mToolTipReason == SliderImmediateContext::ToolTipReason::Hover
    && !w->IsThumbHovered()) {
    ctx->mToolTipReason.reset();
  }

  if (!ctx->mToolTipReason.has_value()) {
    return {w, changed};
  }

  // The slider tooltip is rendered as a sibling of the slider widget in the
  // parent context, taken out of normal flow with PositionType::Absolute and
  // positioned relative to the thumb. This matches Win32's behaviour in
  // spirit — Win32 uses a WS_EX_TRANSPARENT popup that acts as a transparent
  // overlay on the parent — without needing a separate native window. On
  // Linux a separate SDL3 popup couldn't be made reliably mouse-passthrough,
  // so the tooltip would steal cursor events mid-drag and break sliders.
  const bool isHorizontal = (orientation == Orientation::Horizontal);
  const auto value
    = w->IsDragging() ? w->GetSnappedDraggingValue() : w->GetValue();
  const auto pixelOffset = w->GetThumbCenterOffsetWithinTrack();
  const auto trackOrigin = w->GetTrackOriginOffset();

  // Slider's position and size relative to its parent (Yoga values are
  // populated by the previous frame's layout pass, which is fine — the
  // tooltip only appears after a stationary hover or during drag, well
  // after layout).
  const auto* sliderYoga = w->GetLayoutNode();
  const auto sliderLeft = YGNodeLayoutGetLeft(sliderYoga);
  const auto sliderTop = YGNodeLayoutGetTop(sliderYoga);
  const auto sliderWidth = YGNodeLayoutGetWidth(sliderYoga);
  const auto sliderHeight = YGNodeLayoutGetHeight(sliderYoga);

  // Thumb centre in slider-parent coordinates. For vertical sliders,
  // pixelOffset is measured from the bottom (value=min is at the bottom,
  // value=max at the top), so thumbY = sliderTop + sliderHeight - pixelOffset.
  const auto thumbX = isHorizontal
    ? sliderLeft + trackOrigin.mX + pixelOffset
    : sliderLeft + sliderWidth / 2.0f;
  const auto thumbY = isHorizontal
    ? sliderTop + trackOrigin.mY
    : sliderTop + sliderHeight - pixelOffset;

  // Estimated tooltip extents — the tooltip auto-sizes around the label, but
  // PositionType::Absolute needs concrete Top/Left, so we fix the size and
  // accept that very long values would clip. Slider values are usually short.
  static constexpr float TooltipWidth = 64.0f;
  static constexpr float TooltipHeight = 32.0f;
  static constexpr float Gap = 12.0f;

  // For vertical sliders, place the tooltip to the right of the slider.
  // Vertical sliders are typically narrow and often sit flush with the left
  // of their container, so a left-side tooltip would clip off-screen.
  const auto tooltipLeft = isHorizontal
    ? thumbX - TooltipWidth / 2.0f
    : sliderLeft + sliderWidth + Gap;
  const auto tooltipTop = isHorizontal
    ? thumbY - TooltipHeight - Gap
    : thumbY - TooltipHeight / 2.0f;

  const auto tip = BeginWidget<Widget>(
    ID("{}/ToolTip", id.GetValue()),
    LiteralStyleClass {"Slider/ValueToolTip"},
    StaticTheme::ToolTip::DefaultToolTipStyle());
  tip->SetMutableStyles(
    Style()
      .Position(PositionType::Absolute)
      .Left(tooltipLeft)
      .Top(tooltipTop)
      .Width(TooltipWidth)
      .Height(TooltipHeight));
  Label(ctx->FormatValue(w, value), ID {0});
  EndWidget();

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
SliderResult
HSlider(float* pValue, const float minimum, const float maximum, const ID id) {
  return immediate_detail::SliderImpl(
    pValue, minimum, maximum, Orientation::Horizontal, id);
}

SliderResult
VSlider(float* pValue, const float minimum, const float maximum, const ID id) {
  return immediate_detail::SliderImpl(
    pValue, minimum, maximum, Orientation::Vertical, id);
}

}// namespace FredEmmott::GUI::Immediate
