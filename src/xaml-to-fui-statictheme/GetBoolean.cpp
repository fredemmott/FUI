// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetBoolean.hpp"

#include <sstream>

void GetBoolean(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  Resource ret {
    .mName = it.Attribute("x:Key"),
    .mType = "bool",
    .mKind = Resource::Kind::Literal,
  };
  const std::string_view value = it.GetText();
  if (value == "True") {
    ret.mValue = "true";
  } else if (value == "False") {
    ret.mValue = "false";
  } else {
    throw std::logic_error(
      std::format("Don't know how to handle boolean value {}", value));
  }
  back = std::move(ret);
}
