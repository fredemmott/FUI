// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetSolidColorBrush.hpp"
#include "ResolveColorReference.hpp"

void GetAcrylicBrush(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const auto tint = ResolveColorReference(it.Attribute("TintColor"));
  const auto opacity = it.Attribute("TintOpacity");
  const auto fallback = ResolveColorReference(it.Attribute("FallbackColor"));
  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::format(
      "StaticThemedAcrylicBrush {{ {}, {}f, {} }}", tint, opacity, fallback),
    .mType = "Brush",
    .mDependencies = {fallback},
  };
}
