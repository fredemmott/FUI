// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

void Label(std::string_view text, ID id);

template <class... Args>
void Label(std::format_string<Args...> fmt, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(fmt, std::forward<Args>(args)...);
  return Label(text, id);
}

namespace immediate_detail {
using StyleMutator = void (*)(GUI::Style&);

template <StyleMutator... TStyleMutators>
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
    (TStyleMutators(styles), ...);
    GetCurrentNode()->SetAdditionalBuiltInStyles(styles);
  }
};

template <SystemFont::Usage U, StyleMutator... TRest>
struct SystemFontLabel
  : StyledLabel<[](GUI::Style& style) { style.mFont = {U}; }, TRest...> {};

struct Caption : SystemFontLabel<SystemFont::Caption, [](GUI::Style& style) {
  style.mMarginBottom = -4;
}> {};
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