// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetSolidColorBrush.hpp"

#include "ResolveColorReference.hpp"

void GetAcrylicBrush(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const auto value = ResolveColorReference(it.Attribute("FallbackColor"));
  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::format("SolidColorBrush {{ {} }}", value),
    .mType = "Brush",
    .mDependencies = {value},
  };
}
