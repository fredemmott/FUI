// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <optional>
#include <print>

#include "GetSolidColorBrush.hpp"
#include "ResolveColorReference.hpp"
#include "Theme.hpp"

namespace {

// Derived from XamlControlsResources::UpdateAcrylicBrushesDarkTheme
[[nodiscard]]
std::optional<float> GetDarkThemeTintLuminosityOpacity(
  const std::string_view key) {
  if (key.starts_with("Acrylic")) {
    if (key.ends_with("DefaultBrush") || key.ends_with("BaseBrush")) {
      return 0.96f;
    }
    if (key.ends_with("DefaultInverseBrush")) {
      return 0.85f;
    }
  }
  if (key.starts_with("AccentAcrylic")) {
    return 0.8f;
  }

  if (!key.starts_with("System")) {
    std::println(
      stderr,
      "Unhandled AcrylicBrush dark theme TintLuminosityOpacity: {}",
      key);
  }
  return std::nullopt;
}

// Derived from XamlControlsResources::UpdateAcrylicBrushesLightTheme
[[nodiscard]]
std::optional<float> GetLightThemeTintLuminosityOpacity(
  const std::string_view key) {
  if (key.starts_with("Acrylic")) {
    if (key.ends_with("DefaultBrush")) {
      return 0.85f;
    }
    if (key.ends_with("BaseBrush")) {
      return 0.9f;
    }
    if (key.ends_with("DefaultInverseBrush")) {
      return 0.96f;
    }
  }
  if (key.starts_with("AccentAcrylic")) {
    return 0.9f;
  }

  if (!key.starts_with("System")) {
    std::println(
      stderr,
      "Unhandled AcrylicBrush light theme TintLuminosityOpacity: {}",
      key);
  }
  return std::nullopt;
}

[[nodiscard]]
std::optional<float> GetTintLuminosityOpacity(
  const Theme theme,
  const std::string_view key) {
  using enum Theme;
  switch (theme) {
    case Dark:
      return GetDarkThemeTintLuminosityOpacity(key);
    case Light:
      return GetLightThemeTintLuminosityOpacity(key);
    case HighContrast:
    case Unthemed:
      throw std::runtime_error(
        "AcrylicBrush should only be set in Dark or Light themes");
  }
  std::unreachable();
}

}// namespace

void GetAcrylicBrush(
  const Theme theme,
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const auto tint = ResolveColorReference(it.Attribute("TintColor"));
  const auto opacity = it.Attribute("TintOpacity");
  const auto fallback = ResolveColorReference(it.Attribute("FallbackColor"));
  const std::string_view name {it.Attribute("x:Key")};

  const auto tintLuminosityOpacity = GetTintLuminosityOpacity(theme, name);

  if (tintLuminosityOpacity) {
    back = {
      .mName = std::string {name},
      .mValue = std::format(
        "StaticThemedAcrylicBrush {{ {}, {}f, {:f}f, {} }}",
        tint,
        opacity,
        *tintLuminosityOpacity,
        fallback),
      .mType = "Brush",
      .mDependencies = {fallback},
    };
  } else {
    back = {
      .mName = std::string {name},
      .mValue = std::format(
        "StaticThemedAcrylicBrush {{ {}, {}f, std::nullopt, {} }}",
        tint,
        opacity,
        fallback),
      .mType = "Brush",
      .mDependencies = {fallback},
    };
  }
}
