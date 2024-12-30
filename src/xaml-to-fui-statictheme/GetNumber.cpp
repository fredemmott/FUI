// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetNumber.hpp"

void GetNumber(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it,
  const std::string_view type) {
  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = it.GetText(),
    .mType = std::string {type},
    .mKind = Resource::Kind::Literal,
  };
}
