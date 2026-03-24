// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "NavigationViewButton.hpp"

#include "FredEmmott/GUI/StaticTheme/NavigationView.hpp"
#include "FredEmmott/GUI/StaticTheme/TitleBar.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass NavigationViewButtonStyleClass {
  "NavigationView/Button"};
constexpr auto ActiveAnimationDuration
  = StaticTheme::Common::ControlFastAnimationDuration;

const auto& NavigationViewButtonStyle() {
  using namespace StaticTheme::NavigationView;
  static_assert(
    StaticTheme::Generic::NavigationBackButtonWidth
    == StaticTheme::Generic::PaneToggleButtonWidth);
  static_assert(
    StaticTheme::Generic::NavigationBackButtonHeight
    == StaticTheme::Generic::PaneToggleButtonHeight);

  static const ImmutableStyle ret {
    NavigationViewItemStyle()
    + Style()
        .Height(StaticTheme::Generic::PaneToggleButtonHeight)
        .Width(StaticTheme::Generic::PaneToggleButtonWidth)
        .Font(
          SystemFont::ResolveGlyphFont(
            StaticTheme::Generic::ControlContentThemeFontSize))
        .And(
          StaticTheme::TitleBar::TitleBarLeftButtonStyleClass,
          Style().Margin(2).MarginLeft(0).Height(44).Width(
            StaticTheme::TitleBar::TitleBarBackButtonWidth))};
  return ret;
}

}// namespace

NavigationViewButton::NavigationViewButton(const std::string_view glyph)
  : Widget(NavigationViewButtonStyleClass, NavigationViewButtonStyle()),
    IInvocable(this),
    mGlyph(glyph) {
  FUI_ASSERT(!mGlyph.empty());
}

NavigationViewButton::~NavigationViewButton() = default;

void NavigationViewButton::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  FUI_ASSERT(mFont);
  const auto& brush = style.Color().value();

  const auto t = std::chrono::duration_cast<std::chrono::duration<float>>(
                   std::chrono::steady_clock::now() - mAnimationStart)
    / ActiveAnimationDuration;

  // These are private properties which are only set to explicit values, not
  // calculated or set by caller, so a raw float equality check is safe
  FUI_ASSERT(mFromScale == 1.0f || mFromScale == 0.5f);
  FUI_ASSERT(mToScale == 1.0f || mToScale == 0.5f);
  if (t >= 1.0f && mToScale == 1.0f) {
    renderer->DrawText(
      brush, rect, mFont, mGlyph, rect.GetCenter() + mTextOffsetFromCenter);
    return;
  }

  FUI_ASSERT(t >= 0.0f);
  static constexpr auto Ease = EasingFunctions::CubicBezier(
    StaticTheme::Common::ControlFastOutSlowInKeySpline);
  const auto scaleX
    = (t >= 1.0f) ? mToScale : (mFromScale + Ease(t) * (mToScale - mFromScale));
  renderer->PushLayer();
  renderer->Translate(rect.GetCenter());
  // The click animation for the back and hamburger buttons in a navigation view
  // is only a horizontal 'squish' - no vertical changes
  renderer->Scale(scaleX, 1.0f);
  renderer->DrawText(brush, rect, mFont, mGlyph, mTextOffsetFromCenter);
  renderer->PopLayer();
}

Widget::ComputedStyleFlags NavigationViewButton::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  if (const auto font = style.Font().value(); font != mFont) {
    mFont = font;
    const auto metrics = font.GetMetrics();
    mTextOffsetFromCenter = {
      -font.MeasureTextWidth(mGlyph) / 2,
      (metrics.mDescent - metrics.mAscent) / 2,
    };
  }
  return Widget::OnComputedStyleChange(style, state);
}

Widget::EventHandlerResult NavigationViewButton::OnMouseButtonPress(
  const MouseEvent& e) {
  if (
    e.Get<MouseEvent::ButtonPressEvent>().mPressedButtons
    == MouseButton::Left) {
    mFromScale = 1.0f;
    mToScale = 0.5f;
    mAnimationStart = std::chrono::steady_clock::now();
  }
  return Widget::OnMouseButtonPress(e);
}

Widget::EventHandlerResult NavigationViewButton::OnMouseButtonRelease(
  const MouseEvent& e) {
  if (
    e.Get<MouseEvent::ButtonReleaseEvent>().mReleasedButtons
    == MouseButton::Left) {
    FUI_ASSERT(mToScale == 0.5f);
    mFromScale = 0.5f;
    mToScale = 1.0f;
    mAnimationStart = std::chrono::steady_clock::now();
  }
  return Widget::OnMouseButtonRelease(e);
}

Widget::EventHandlerResult NavigationViewButton::OnClick(const MouseEvent&) {
  this->Invoke();
  return EventHandlerResult::StopPropagation;
}

FrameRateRequirement NavigationViewButton::GetFrameRateRequirement()
  const noexcept {
  if (
    std::chrono::steady_clock::now() - mAnimationStart
    <= ActiveAnimationDuration) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return Widget::GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Widgets