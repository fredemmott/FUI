// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "GetDesktopAcrylicBackdrop.hpp"

void GetDesktopAcrylicBackdrop(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  static constexpr std::string_view Type {
    "::FredEmmott::GUI::WindowBackdrops::Win32_DesktopAcrylic"};
  static constexpr std::string_view Brush {
    "AcrylicBackgroundFillColorDefaultBrush"};
  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::format("{} {{ {} }}", Type, Brush),
    .mType = std::string {Type},
    .mDependencies = {std::string {Brush}},
  };
}