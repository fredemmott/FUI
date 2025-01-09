// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/assert.hpp>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

bool ComboBox(
  std::size_t* selectedIndex,
  std::span<std::string_view> items,
  ID id = ID {std::source_location::current()});

template <class T>
bool ComboBox(
  T* selectedKey,
  const std::ranges::range auto& items,
  ID id = ID {std::source_location::current()})
  requires requires {
    { std::get<0>(*std::ranges::cbegin(items)) } -> std::convertible_to<T>;
    {
      std::get<1>(*std::ranges::cbegin(items))
    } -> std::convertible_to<std::string_view>;
  }
{
  FUI_ASSERT(selectedKey, "A key is always required");
  std::vector<std::string_view> vec;
  vec.reserve(vec.size());
  std::size_t selectedIndex = 0;
  for (auto&& [key, value]: items) {
    if (key == *selectedKey) {
      selectedIndex = vec.size();
    }
    vec.push_back(std::string_view {value});
  }
  const auto changed = ComboBox(&selectedIndex, vec, id);
  if (changed) {
    *selectedKey
      = *(std::ranges::cbegin(std::views::keys(items)) + selectedIndex);
  }
  return changed;
}

template <class T>
bool ComboBox(
  T* selectedKey,
  const std::ranges::range auto& items,
  ID id = ID {std::source_location::current()})
  requires requires {
    { *std::ranges::cbegin(items) } -> std::convertible_to<std::string_view>;
  }
{
  return ComboBox(selectedKey, std::views::enumerate(items), id);
}

}// namespace FredEmmott::GUI::Immediate