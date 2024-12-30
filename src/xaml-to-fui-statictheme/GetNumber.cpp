// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetNumber.hpp"

#include <sstream>

void GetNumber(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  Resource ret {
    .mName = it.Attribute("x:Key"),
    .mValue = it.GetText(),
    .mKind = Resource::Kind::Literal,
  };

  const auto type = it.ValueStr();

  if (type == "x:Double") {
    ret.mType = "double";
    back = std::move(ret);
    return;
  }

  if (type == "x:Int32") {
    ret.mType = "int32_t";
    back = std::move(ret);
    return;
  }

  throw std::logic_error(
    std::format("Don't know how to handle number type {}", type));
}
