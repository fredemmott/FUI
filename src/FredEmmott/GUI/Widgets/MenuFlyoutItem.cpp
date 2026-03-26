// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "MenuFlyoutItem.hpp"

#include "FredEmmott/GUI/StaticTheme/MenuFlyout.hpp"
#include "FredEmmott/GUI/StaticTheme/detail/NavigationView.handwritten.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass MenuFlyoutItemStyleClass {"MenuFlyoutItem"};
constexpr LiteralStyleClass MenuFlyoutItemLabelStyleClass {
  "MenuFlyoutItem/Label"};

constexpr auto GlyphSize = StaticTheme::MenuFlyout::MenuFlyoutIconSize;

const ImmutableStyle& MenuFlyoutItemLabelStyle() {
  static const ImmutableStyle ret {
    Style().Margin(
      StaticTheme::MenuFlyout::MenuFlyoutItemPlaceholderThemeThickness),
  };
  return ret;
}

}// namespace

MenuFlyoutItem::MenuFlyoutItem(Window* window)
  : Button(
      window,
      MenuFlyoutItemStyleClass,
      StaticTheme::MenuFlyout::MenuFlyoutItemStyle()),
    mGlyphFont(SystemFont::ResolveGlyphFont(GlyphSize)),
    mLabel(new Label(
      window,
      MenuFlyoutItemLabelStyleClass,
      MenuFlyoutItemLabelStyle())) {
  this->SetStructuralChildren({mLabel});

  using namespace StaticTheme::MenuFlyout;
  const auto metrics = mGlyphFont.GetMetrics();

  mGlyphOffset = {
    (-mGlyphFont.MeasureTextWidth(mGlyph) / 2)
      + MenuFlyoutItemThemePadding.GetLeft() + MenuFlyoutItemBorderThickness,
    (metrics.mDescent - metrics.mAscent) / 2
      + MenuFlyoutItemThemePadding.GetTop() + MenuFlyoutItemBorderThickness,
  };
}

MenuFlyoutItem::~MenuFlyoutItem() = default;

void MenuFlyoutItem::SetGlyph(const std::string_view glyph) {
  mGlyph = glyph;
}

void MenuFlyoutItem::SetLabel(const std::string_view label) {
  mLabel->SetText(label);
}

void MenuFlyoutItem::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  Button::PaintOwnContent(renderer, rect, style);

  if (mGlyph.empty()) {
    return;
  }

  const auto glyphPos = rect.GetTopLeft() + mGlyphOffset
    + Point {
      0,
      (rect.GetHeight() - mGlyphFont.GetMetrics().mSize) / 2,
    };
  renderer->DrawText(*style.Color(), rect, mGlyphFont, mGlyph, glyphPos);
}

}// namespace FredEmmott::GUI::Widgets