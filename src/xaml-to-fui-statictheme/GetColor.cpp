// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "GetColor.hpp"

#include "GetHexColorValue.hpp"

void GetColor(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = GetHexColorValue(it.GetText()),
    .mType = "Color",
  };
}
