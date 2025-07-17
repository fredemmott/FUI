// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <type_traits>

namespace FredEmmott::GUI::Immediate {
/** An identifier for the selected item in a set of radio buttons.
 *
 * This is primarily intended to allow ints, `std::size_t`, and
 * enums, but given all we need to allow is equality, it's fairly
 * permissive.
 *
 * - Pointers are disallowed to avoid confusion for parameters that
 *   take a pointer
 * - floats are disallowed as while they support ==, using == is
 *   usually a bad idea
 */
template <class T>
concept selectable_key = std::equality_comparable<T>
  && std::is_trivially_copyable_v<T> && (!std::is_pointer_v<T>)
  && (!std::is_void_v<T>) && (!std::is_floating_point_v<T>);
}// namespace FredEmmott::GUI::Immediate