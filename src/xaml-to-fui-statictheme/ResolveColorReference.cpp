// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ResolveColorReference.hpp"

#include <format>
#include <regex>

#include "GetHexColorValue.hpp"

std::string ResolveColorReference(std::string_view value) {
  if (value.starts_with('#')) {
    return GetHexColorValue(value);
  }

  constexpr std::string_view themePrefix {"{ThemeResource "};
  constexpr std::string_view brushSuffix {"Brush}"};

  if (value.starts_with(themePrefix) && value.ends_with(brushSuffix)) {
    value.remove_prefix(themePrefix.size());
    value.remove_suffix(brushSuffix.size());
    return std::string {value};
  }

  if (
    value.starts_with(themePrefix) && value.ends_with('}')
    && value.contains("Color")) {
    value.remove_prefix(themePrefix.size());
    value.remove_suffix(1);
    return std::string {value};
  }

  constexpr std::string_view staticPrefix {"{StaticResource "};
  if (
    value.starts_with(staticPrefix) && value.ends_with('}')
    && value.contains("Color")) {
    value.remove_prefix(staticPrefix.size());
    value.remove_suffix(1);
    return std::string {value};
  }

  if (value == "LightGray") {
    return "Colors::LightGray";
  }

  static const std::regex NamedColor(
    "^[A-Z][a-zA-Z]+$", std::regex_constants::extended);
  if (std::regex_match(value.begin(), value.end(), NamedColor)) {
    return std::format("Colors::{}", value);
  }

  throw std::runtime_error {
    std::format("Can't figure out how to resolve color reference `{}`", value)};
}
