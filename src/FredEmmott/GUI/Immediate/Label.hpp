// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

void Label(std::string_view text, ID id = ID {std::source_location::current()});

template <class... Args>
void Label(std::format_string<Args...> fmt, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(fmt, std::forward<Args>(args)...);
  return Label(text, id);
}

namespace immediate_detail {

template <class TDerived>
struct StyledLabel {
  static void operator()(
    const std::string_view text,
    const ID id = ID {std::source_location::current()}) {
    Label(text, id);
    ApplyStyles();
  }

  template <class... Args>
  static void operator()(std::format_string<Args...> fmt, Args&&... args) {
    Label(fmt, std::forward<Args>(args)...);
    ApplyStyles();
  }

 private:
  static void ApplyStyles() {
    GUI::Style styles;
    TDerived::ApplyStyles(styles);
    GetCurrentNode()->SetAdditionalBuiltInStyles(styles);
  }
};

template <SystemFont::Usage U>
struct SystemFontLabel : StyledLabel<SystemFontLabel<U>> {
  static void ApplyStyles(Style& style) {
    style.mFont = {U};
  }
};

struct Caption : StyledLabel<Caption> {
  static void ApplyStyles(Style& style) {
    SystemFontLabel<SystemFont::Caption>::ApplyStyles(style);
    style.mMarginBottom = -4;
  }
};
}// namespace immediate_detail

constexpr auto CaptionLabel
  = immediate_detail::SystemFontLabel<SystemFont::Caption> {};
constexpr auto BodyLabel
  = immediate_detail::SystemFontLabel<SystemFont::Caption> {};
constexpr auto BodyStrongLabel
  = immediate_detail::SystemFontLabel<SystemFont::BodyStrong> {};
constexpr auto BodyLargeLabel
  = immediate_detail::SystemFontLabel<SystemFont::BodyLarge> {};
constexpr auto SubtitleLabel
  = immediate_detail::SystemFontLabel<SystemFont::Subtitle> {};
constexpr auto TitleLabel
  = immediate_detail::SystemFontLabel<SystemFont::Title> {};
constexpr auto TitleLargeLabel
  = immediate_detail::SystemFontLabel<SystemFont::TitleLarge> {};
constexpr auto DisplayLabel
  = immediate_detail::SystemFontLabel<SystemFont::Display> {};

// A CaptionLabel with the margin adjusted to be closer to the following widget
constexpr auto Caption = immediate_detail::Caption {};

}// namespace FredEmmott::GUI::Immediate