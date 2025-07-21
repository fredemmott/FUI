// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetSolidColorBrush.hpp"

#include "ResolveColorReference.hpp"

void GetSolidColorBrush(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const auto value = ResolveColorReference(it.Attribute("Color"));
  auto stringValue = std::format("SolidColorBrush {{ {} }}", value);
  if (const auto opacity = it.Attribute("Opacity")) {
    stringValue += std::format(".WithAlphaMultipliedBy({})", opacity);
  }
  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = stringValue,
    .mType = "Brush",
    .mDependencies = {value},
  };
}
