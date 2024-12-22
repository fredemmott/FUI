// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Color.hpp"
#include "Font.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI {

class Label final : public Widget {
 public:
  struct Options {
    Font mFont {SystemFont::Body};
    Color mColor {SystemColor::Foreground};
  };

  Label(std::size_t id, const Options&, std::string_view text);
  void Paint(SkCanvas*) const override;

 private:
  Options mOptions;
  std::string mText;

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI
