// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <format>

#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"

namespace FredEmmott::GUI {

class Label final : public Widget {
 public:
  struct Options {
    Font mFont { SystemFont::Usage::Body };
    Color mColor { SystemColor::Usage::Foreground };
  };

  template <class... Args>
  explicit Label(const Options& options, std::format_string<Args...> format, Args&&... args)
    : Label(internal_t {}, options, format, std::format(format, std::forward<Args>(args)...)) {
  }

  void Paint(SkCanvas*) const override;

 private:
  struct internal_t {};
  Label(internal_t, const Options& options, auto format, const std::string_view formatted)
    : Widget(FormattedToId(format, formatted)), mOptions(options), mText(formatted) {
    this->SetLayoutConstraints();
  }

  Options mOptions;
  std::string mText;

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI
