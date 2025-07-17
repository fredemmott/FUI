// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <ranges>
#include <span>
#include <string>
#include <vector>

#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "ID.hpp"
#include "Result.hpp"
#include "selectable_key.hpp"

namespace FredEmmott::GUI::Immediate {
template <class TValue = void>
using ComboBoxResult
  = Result<nullptr, TValue, immediate_detail::CaptionResultMixin>;

ComboBoxResult<bool> ComboBox(
  std::size_t* selectedIndex,
  std::span<const std::string_view> items,
  ID id = ID {std::source_location::current()});

template <std::convertible_to<std::span<const std::string_view>> TContainer>
ComboBoxResult<bool> ComboBox(
  std::size_t* selectedIndex,
  TContainer&& items,
  const ID id = ID {std::source_location::current()}) {
  return ComboBox(
    selectedIndex,
    std::span<const std::string_view> {std::forward<TContainer>(items)},
    id);
}

template <
  std::integral TIndex,
  std::convertible_to<std::span<const std::string_view>> TContainer>
ComboBoxResult<bool> ComboBox(
  TIndex* selectedIndex,
  TContainer&& items,
  const ID id = ID {std::source_location::current()}) {
  auto buf = static_cast<std::size_t>(*selectedIndex);
  const auto ret
    = ComboBox(&buf, std::span {std::forward<TContainer>(items)}, id);
  *selectedIndex = static_cast<TIndex>(buf);
  return ret;
}

/** Produce a combobox from a container of items
 *
 * For example, the default projection supports an `std::vector<const char*>`
 * or anything similar - but a custom projection can be used, e.g.
 * `&MyItemClass::mLabel`
 */
template <std::integral TIndex, class TLabelProj = std::identity>
ComboBoxResult<bool> ComboBox(
  TIndex* selectedIndex,
  const std::ranges::range auto& items,
  TLabelProj labelProj = {},
  ID id = ID {std::source_location::current()})
  requires requires {
    {
      std::invoke(labelProj, *std::ranges::cbegin(items))
    } -> std::convertible_to<std::string_view>;
  }
{
  const auto labels = items | std::views::transform([&labelProj](auto&& it) {
                        return std::string_view {std::invoke(labelProj, it)};
                      })
    | std::ranges::to<std::vector>();

  return ComboBox(selectedIndex, labels, id);
}

template <std::size_t N>
struct nth_element {
  template <class T>
    requires requires(std::decay_t<T> v) { std::get<N>(v); }
  static decltype(auto) operator()(T&& it) {
    return std::get<N>(std::forward<T>(it));
  }
};

/** Produce a combobox from a container, and a set of key-value projections.
 *
 * Keys are not required to be contiguous.
 *
 * The default projections allow use with keyed containers, like an `std::map`
 */
template <
  selectable_key T,
  class TLabelProj = nth_element<1>,
  class TKeyProj = nth_element<0>>
ComboBoxResult<bool> ComboBox(
  T* selectedKey,
  const std::ranges::range auto& items,
  TLabelProj labelProj = {},
  TKeyProj keyProj = {},
  const ID id = ID {std::source_location::current()})
  requires requires {
    {
      std::invoke(keyProj, *std::ranges::cbegin(items))
    } -> std::convertible_to<T>;
    {
      std::invoke(labelProj, *std::ranges::cbegin(items))
    } -> std::convertible_to<std::string_view>;
  }
{
  FUI_ASSERT(selectedKey, "A key is always required");
  std::vector<std::string_view> vec;
  vec.reserve(vec.size());
  std::size_t selectedIndex = 0;
  for (auto&& it: items) {
    const auto key = std::invoke(keyProj, it);
    const auto value = std::string_view {std::invoke(labelProj, it)};
    if (key == *selectedKey) {
      selectedIndex = vec.size();
    }
    vec.push_back(std::string_view {value});
  }
  const auto changed = ComboBox(&selectedIndex, vec, id);
  if (changed) {
    *selectedKey = std::invoke(
      keyProj, *std::next(std::ranges::cbegin(items), +selectedIndex));
  }
  return changed;
}

}// namespace FredEmmott::GUI::Immediate