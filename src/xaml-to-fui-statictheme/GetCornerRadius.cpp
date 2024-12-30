// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <ranges>

#include "GetThickness.hpp"

void GetCornerRadius(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  std::string_view text {it.GetText()};
  const auto parts
    = text | std::views::split(',') | std::views::transform([](auto&& range) {
        return std::string_view(&*range.begin(), std::ranges::distance(range));
      })
    | std::ranges::to<std::vector>();

  const auto value = parts[0];
  switch (parts.size()) {
    case 1:
      break;
    case 4: {
      auto b = parts[1];
      auto c = parts[2];
      auto d = parts[3];
      if (value != b || value != c || value != d) {
        throw std::runtime_error(
          std::format(
            "<CornerRadius> value `{}` has differing components", text));
      }
      break;
    }
  }

  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::string {value},
    .mType = "double",
    .mKind = Resource::Kind::Literal,
  };
}
