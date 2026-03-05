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

  const auto Push = [&back](
                      const std::string_view key,
                      const std::string_view value,
                      const std::string_view type = "UniformCornerRadius") {
    back = {
      .mName = std::string {key},
      .mValue = std::string {value},
      .mType = std::string {type},
      .mKind = Resource::Kind::Literal,
    };
  };
  const std::string_view key = it.Attribute("x:Key");

  const auto value = parts[0];
  switch (parts.size()) {
    case 1:
      Push(key, value);
      return;
    case 4: {
      const auto b = parts[1];
      const auto c = parts[2];
      const auto d = parts[3];
      if (value == b && value == c && value == d) {
        Push(key, value);
        return;
      }
      Push(key, text, "CornerRadius");
      break;
    }
    default:
      throw std::runtime_error(
        std::format(
          "<CornerRadius> value `{}` has unhandled number of values", text));
  }
}
