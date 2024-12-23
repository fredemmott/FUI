// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <type_traits>

namespace FredEmmott::concepts {

template <class T>
concept scoped_enum = std::is_scoped_enum_v<T>;

template <class T>
concept any_enum = std::is_enum_v<T>;

template <class T>
concept unscoped_enum = any_enum<T> && !scoped_enum<T>;

}// namespace FredEmmott::concepts