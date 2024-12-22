// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <format>

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

  template <class... Args>
  Label(
    const Options& options,
    std::format_string<Args...> format,
    Args&&... args)
    : Label(options, FormattedLabel {format, std::forward<Args>(args)...}) {
  }

  void Paint(SkCanvas*) const override;

 private:
  Label(const Options& options, const FormattedLabel& label)
    : Widget(label.mID), mOptions(options), mText(label.mText) {
    this->SetLayoutConstraints();
  }

  Options mOptions;
  std::string mText;

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI
