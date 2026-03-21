// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <type_traits>

namespace FredEmmott::GUI::Immediate {
/** An identifier for the selected item in a group.
 *
 * This includes widgets like:
 *
 * - radio buttons
 * - navigation items
 *
 * This is primarily intended to allow ints, `std::size_t`, and
 * enums, but given all we need to allow is equality, it's fairly
 * permissive, e.g. you could use an `std::variant<empty_marker_t, indexed_t>`
 *
 * - Pointers are disallowed to avoid confusion for parameters that
 *   take a pointer
 * - floats are disallowed as while they support ==, using == is
 *   usually a bad idea
 *
 *  You could, for example, use a variant to distinguish between user-generated
 *  navigation items and 'special' navigation items like settings.
 */
template <class T>
concept selectable_key = std::equality_comparable<T>
  && std::is_trivially_copyable_v<T> && (!std::is_pointer_v<T>)
  && (!std::is_void_v<T>) && (!std::is_floating_point_v<T>);
}// namespace FredEmmott::GUI::Immediate