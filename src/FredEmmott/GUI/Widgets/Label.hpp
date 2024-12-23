// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "../Color.hpp"
#include "../Font.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Label final : public Widget {
 public:
  struct Options {
    Font mFont {SystemFont::Body};
    Color mColor {SystemColor::Foreground};
  };

  Label(std::size_t id, const Options&);
  void Paint(SkCanvas*) const override;

  std::string_view GetText() const noexcept {
    return mText;
  }
  void SetText(std::string_view);

 private:
  Options mOptions;
  std::string mText;

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI::Widgets
