// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetThickness.hpp"

#include <algorithm>
#include <ranges>

void GetThickness(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  std::string_view text {it.GetText()};
  const auto parts
    = text | std::views::split(',') | std::views::transform([](auto&& range) {
        return std::string_view(&*range.begin(), std::ranges::distance(range));
      })
    | std::ranges::to<std::vector>();

  uint8_t topIndex = 0;
  uint8_t rightIndex = 0;
  uint8_t bottomIndex = 0;
  switch (parts.size()) {
    case 1:
      break;
    case 2:
      topIndex = 1;
      rightIndex = 0;
      bottomIndex = 1;
      break;
    case 4:
      topIndex = 1;
      rightIndex = 2;
      bottomIndex = 3;
      break;
    default:
      throw std::runtime_error(
        std::format(
          "<Thickness> value `{}` has an unhandled number of components ({})",
          text,
          parts.size()));
  }

  const auto l = parts[0];
  const auto t = parts[topIndex];
  const auto r = parts[rightIndex];
  const auto b = parts[bottomIndex];

  const auto key = std::string_view {it.Attribute("x:Key")};

  if (
    (parts.size() == 1) && (key.contains("Border") || key.contains("Stroke"))) {
    // FUI's `Style` class only supports uniform thickness for these, so let's
    // expose a uniform property.
    back = {
      .mName = std::string {key},
      .mValue = std::string {l},
      .mType = "float",
      .mKind = Resource::Kind::Literal,
    };
    return;
  }

  for (const auto& [suffix, value]: {
         std::tuple {
           "Left",
           l,
         },
         std::tuple {
           "Top",
           t,
         },
         std::tuple {
           "Right",
           r,
         },
         std::tuple {
           "Bottom",
           b,
         },
       }) {
    back = {
      .mName = std::format("{}{}", key, suffix),
      .mValue = std::string {value},
      .mType = "float",
      .mKind = Resource::Kind::Literal,
    };
  }
}
