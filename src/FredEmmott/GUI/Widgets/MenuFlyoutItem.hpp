// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "Button.hpp"

namespace FredEmmott::GUI::Widgets {

class Label;

class MenuFlyoutItem : public Button {
 public:
  explicit MenuFlyoutItem(Window*);
  ~MenuFlyoutItem() override;

  void SetGlyph(std::string_view);
  void SetLabel(std::string_view);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  Font mGlyphFont;
  Font mLabelFont {};
  std::string mGlyph;
  Label* mLabel {};
  Point mGlyphOffset {};
  Point mLabelOffset {};
};

}// namespace FredEmmott::GUI::Widgets