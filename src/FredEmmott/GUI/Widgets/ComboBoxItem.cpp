// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include "FredEmmott/GUI/StaticTheme/ComboBox.hpp"
#include "FredEmmott/GUI/StaticTheme/Common.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {

constexpr LiteralStyleClass ComboBoxItemButtonStyleClass("ComboBox/ItemButton");

Style MakeComboBoxItemStyles() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;

  const auto BaseStyles
    = Style()
        .BackgroundColor(ComboBoxItemBackground)
        .BorderColor(ComboBoxItemBorderBrush)
        .BorderRadius(ComboBoxItemCornerRadius)
        .Color(ComboBoxItemForeground)
        .FlexDirection(FlexDirection::Row)
        .Gap(0)
        .MarginBottom(2)
        .MarginLeft(5)
        .MarginRight(5)
        .MarginTop(2)
        .Padding(ComboBoxItemThemePadding)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundDisabled)
            .BorderColor(ComboBoxItemBorderBrushDisabled)
            .Color(ComboBoxItemForegroundDisabled))
        .And(
          Hover,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundPointerOver)
            .BorderColor(ComboBoxItemBorderBrushPointerOver)
            .Color(ComboBoxItemForegroundPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundPressed)
            .BorderColor(ComboBoxItemBorderBrushPressed)
            .Color(ComboBoxItemForegroundPressed));
  const auto SelectedStyles
    = Style()
        .BackgroundColor(ComboBoxItemBackgroundSelected)
        .BorderColor(ComboBoxItemBorderBrushSelected)
        .Color(ComboBoxItemForegroundSelected)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundSelectedDisabled)
            .BorderColor(ComboBoxItemBorderBrushSelectedDisabled)
            .Color(ComboBoxItemForegroundSelectedDisabled))
        .And(
          Hover,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundSelectedPointerOver)
            .BorderColor(ComboBoxItemBorderBrushSelectedPointerOver)
            .Color(ComboBoxItemForegroundSelectedPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundSelectedPressed)
            .BorderColor(ComboBoxItemBorderBrushSelectedPressed)
            .Color(ComboBoxItemForegroundSelectedPressed));

  return BaseStyles + Style().And(Checked, SelectedStyles);
}

auto& ComboBoxItemStyles() {
  static const ImmutableStyle ret {MakeComboBoxItemStyles()};
  return ret;
}

}// namespace

ComboBoxItem::ComboBoxItem(Window* const window)
  : Button(window, ComboBoxItemButtonStyleClass, ComboBoxItemStyles()),
    ISelectionItem(this) {
  using namespace StaticTheme::ComboBox;
  mSelectionPill.SetSelectedPressedAnimation(
    ComboBoxItemPillMinScale, ComboBoxItemScaleAnimationDuration);
}

ComboBoxItem::~ComboBoxItem() = default;

void ComboBoxItem::Tick(const std::chrono::steady_clock::time_point& now) {
  Button::Tick(now);
  mSelectionPill.Tick(now);

  if (
    IsChecked()
    && mSelectionPill.GetState() == SelectionPill::State::NotSelected) {
    mSelectionPill.Transition(SelectionPill::State::Selected);
  }
}
void ComboBoxItem::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  using namespace StaticTheme::ComboBox;

  Button::PaintOwnContent(renderer, rect, style);

  const Rect pillRect {
    rect.GetTopLeft(),
    Size {
      ComboBoxItemPillWidth,
      rect.GetHeight(),
    },
  };

  const auto theme = StaticTheme::GetCurrent();

  mSelectionPill.Paint(
    renderer,
    pillRect,
    ComboBoxItemPillFillBrush.Resolve(theme),
    ComboBoxItemPillHeight);
}

Widget::EventHandlerResult ComboBoxItem::OnMouseButtonPress(
  const MouseEvent& e) {
  if (IsChecked()) {
    mSelectionPill.Transition(SelectionPill::State::SelectedPressed);
  }
  return Button::OnMouseButtonPress(e);
}

void ComboBoxItem::OnMouseEnter(const MouseEvent& e) {
  Button::OnMouseEnter(e);
  if (
    mSelectionPill.GetState() == SelectionPill::State::SelectedReleased
    && (e.mButtons == MouseButton::Left)) {
    // WinUI3 doesn't do this, but it feels better like this :)
    mSelectionPill.Transition(SelectionPill::State::SelectedPressed);
  }
}

void ComboBoxItem::OnMouseLeave(const MouseEvent& e) {
  Button::OnMouseLeave(e);
  if (mSelectionPill.GetState() == SelectionPill::State::SelectedPressed) {
    mSelectionPill.Transition(SelectionPill::State::SelectedReleased);
  }
}

FrameRateRequirement ComboBoxItem::GetFrameRateRequirement() const noexcept {
  return Button::GetFrameRateRequirement()
    + mSelectionPill.GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Widgets
