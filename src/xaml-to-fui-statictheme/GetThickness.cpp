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
  const auto uniform = parts.size() == 1;
  if ((!uniform) && parts.size() != 4) {
    throw std::runtime_error(
      std::format(
        "<Thickness> value `{}` has an unhandled number of components", text));
  }

  const auto l = parts[0];
  const auto t = parts[uniform ? 0 : 1];
  const auto r = parts[uniform ? 0 : 2];
  const auto b = parts[uniform ? 0 : 3];

  const auto key = std::string_view {it.Attribute("x:Key")};

  if (uniform && (key.contains("Border") || key.contains("Stroke"))) {
    // FUI's `Style` class only supports uniform thickness for these, so let's
    // expose a uniform property.
    back = {
      .mName = std::string {key},
      .mValue = std::string {l},
      .mType = "double",
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
      .mType = "double",
      .mKind = Resource::Kind::Literal,
    };
  }
}
