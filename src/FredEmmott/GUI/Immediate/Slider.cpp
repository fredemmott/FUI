// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/ToolTip.hpp>
#include <stdexcept>
#include <utility>

#include "PopupWindow.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {

struct SliderImmediateContext : Widgets::Context {
  ImmutableStyle mRootStyle;
  ImmutableStyle mCenteringContainerStyle;
};

[[nodiscard]]
SliderResult
SliderImpl(float* const pValue, const Orientation orientation, const ID id) {
  using namespace immediate_detail;
  if (!pValue) [[unlikely]] {
    throw std::logic_error("Slider requires a non-null value pointer");
  }

  const auto w = ChildlessWidget<Widgets::Slider>(id, orientation);

  const auto changed = std::exchange(w->mChanged, false);
  if (changed) {
    *pValue = w->GetValue();
  } else {
    w->SetValue(*pValue);
  }

  if (w->IsDragging()) {
    using namespace StaticTheme::ToolTip;
    static constexpr auto VerticalPadding = 32;
    const auto parentWindow = tWindow;
    const bool isHorizontal = (orientation == Orientation::Horizontal);
    const auto ctx = w->GetOrCreateContext<SliderImmediateContext>();

    BeginBasicPopupWindow(ID("{}/Popup", id.GetValue())).Transparent();
    if (!tWindow->GetNativeHandle().mValue) {
      const auto width = ToolTipMaxWidth
        + (isHorizontal ? YGNodeLayoutGetWidth(w->GetLayoutNode()) : 0);
      const auto height = isHorizontal
        ? std::numeric_limits<float>::quiet_NaN()
        : (YGNodeLayoutGetHeight(w->GetLayoutNode())
           + ToolTipContentThemeFontSize + (2 * VerticalPadding));
      const auto windowOffset = isHorizontal
        ? Point {ToolTipMaxWidth / 2, 56}
        : Point {ToolTipMaxWidth + 12, VerticalPadding};
      const auto windowOrigin = isHorizontal
        ? (w->GetTrackOriginOffset() + w->GetTopLeftCanvasPoint()
           - windowOffset)
        : (w->GetTopLeftCanvasPoint() - windowOffset);

      tWindow->SetIsToolTip();
      tWindow->SetInitialPositionInNativeCoords(
        parentWindow->CanvasPointToNativePoint(windowOrigin));
      ctx->mRootStyle = ImmutableStyle {
        Style().Height(height).Width(width)
        //.BackgroundColor(Color::Constant::FromRGBA32(0x550000FF))
      };
      ctx->mCenteringContainerStyle = ImmutableStyle {
        Style()
          .PaddingTop(
            isHorizontal ? 0 : VerticalPadding + w->GetTrackOriginOffset().mY)
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

    // 2. Centering container: Probably bigger than the content, just centers it
    // Because of the relationship between the two widths, this is centered over
    // the offset, which we mutate with TranslateX
    const auto inner = BeginWidget<Widget>(
      ID {0},
      LiteralStyleClass {"Slider/ValueToolTip/CenteringContainer"},
      ctx->mCenteringContainerStyle);
    if (isHorizontal) {
      inner->SetMutableStyles(Style().TranslateX(w->GetDraggingTrackOffset()));
    } else {
      inner->SetMutableStyles(Style().TranslateY(-w->GetDraggingTrackOffset()));
    }
    // 3. The actual tooltip: styling only, adjusts to size of label inside
    BeginWidget<Widget>(
      ID {0}, LiteralStyleClass {"Slider/ValueToolTip"}, DefaultToolTipStyle());

    // 4. The text :)
    Label(std::to_string(w->GetSnappedDraggingValue()), ID {0});

    EndWidget();
    EndWidget();
    EndWidget();
    EndBasicPopupWindow();
  }

  return {w, changed};
}
}// namespace

SliderResult HSlider(float* pValue, const ID id) {
  return SliderImpl(pValue, Orientation::Horizontal, id);
}

SliderResult VSlider(float* pValue, const ID id) {
  return SliderImpl(pValue, Orientation::Vertical, id);
}

}// namespace FredEmmott::GUI::Immediate