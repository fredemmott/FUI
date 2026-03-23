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

  const auto key = std::string_view {it.Attribute("x:Key")};
  switch (parts.size()) {
    case 1:
      back = {
        .mName = std::string {key},
        .mValue = std::string {text},
        .mType = "float",
        .mKind = Resource::Kind::Literal,
      };
      return;
    case 2:
      // XAML: `horizontal vertical`
      // CSS (and 'Edges<T>'): `vertical horizontal`
      back = {
        .mName = std::string {key},
        .mValue = std::format("{{ {}, {} }}", parts[1], parts[0]),
        .mType = "Edges<float>",
        .mKind = Resource::Kind::BracedLiteral,
      };
      return;
    case 4:
      // XAML: `left top right bottom`
      // CSS (and 'Edges<T>'): `top right bottom left`
      back = {
        .mName = std::string {key},
        .mValue = std::format(
          "{{ {}, {}, {}, {} }}", parts[1], parts[2], parts[3], parts[0]),
        .mType = "Edges<float>",
        .mKind = Resource::Kind::BracedLiteral,
      };
      return;
    default:
      throw std::runtime_error(
        std::format(
          "<Thickness> value `{}` has an unhandled number of components ({})",
          text,
          parts.size()));
  }
}
