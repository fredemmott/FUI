// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBar.hpp"

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>
#include <FredEmmott/GUI/detail/ScrollBarStyles.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

ScrollBar::ScrollBar(std::size_t id, Orientation orientation)
  : Widget(
      id,
      {ScrollBarStyleClass(),
       (orientation == Orientation::Horizontal
          ? ScrollBarHorizontalStyleClass()
          : ScrollBarVerticalStyleClass())}),
    mOrientation(orientation) {
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  constexpr auto leftGlyph = "\uedd9";
  constexpr auto rightGlyph = "\uedda";
  constexpr auto upGlyph = "\ueddb";
  constexpr auto downGlyph = "\ueddc";

  this->ChangeDirectChildren([this] {
    mSmallDecrement.reset(new Label(
      0,
      {ScrollBarSmallChangeStyleClass(), ScrollBarSmallDecrementStyleClass()}));
    mLargeDecrement.reset(new Widget(
      0,
      {ScrollBarLargeChangeStyleClass(), ScrollBarLargeDecrementStyleClass()}));
    mThumb.reset(new Widget(0, {ScrollBarThumbStyleClass()}));
    mLargeIncrement.reset(new Widget(
      0,
      {ScrollBarLargeChangeStyleClass(), ScrollBarLargeIncrementStyleClass()}));
    mSmallIncrement.reset(new Label(
      0,
      {ScrollBarSmallChangeStyleClass(), ScrollBarSmallIncrementStyleClass()}));
  });

  mSmallDecrement->ReplaceBuiltInStyleSheet(ScrollBarSmallChangeStyles());
  mSmallIncrement->ReplaceBuiltInStyleSheet(ScrollBarSmallChangeStyles());
  mLargeDecrement->ReplaceBuiltInStyleSheet(ScrollBarLargeChangeStyles());
  mLargeIncrement->ReplaceBuiltInStyleSheet(ScrollBarLargeChangeStyles());
  mThumb->ReplaceBuiltInStyleSheet(ScrollBarThumbStyles());

  switch (orientation) {
    case Orientation::Vertical:
      mSmallDecrement->SetText(upGlyph);
      mSmallIncrement->SetText(downGlyph);
      break;
    case Orientation::Horizontal:
      mSmallDecrement->SetText(leftGlyph);
      mSmallIncrement->SetText(rightGlyph);
      break;
  }
}

ScrollBar::~ScrollBar() = default;

WidgetList ScrollBar::GetDirectChildren() const noexcept {
  return {
    mSmallDecrement.get(),
    mLargeDecrement.get(),
    mThumb.get(),
    mLargeIncrement.get(),
    mSmallIncrement.get(),
  };
}

}// namespace FredEmmott::GUI::Widgets