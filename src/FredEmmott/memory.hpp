// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "memory/detail.hpp"

namespace FredEmmott::Memory {

/** An `std::unique_ptr` taking a `void(T*)` deleter function.
 *
 * If `nullptr` is provided, `std::default_delete<T{}>()` will be used.
 */
template <class T, auto TDeleter = nullptr>
using unique_ptr = std::unique_ptr<T, detail::deleter_type_t<T, TDeleter>>;

/** An `std::shared_ptr` taking a `void(T*)` deleter function.
 *
 * If `nullptr` is provided, `std::default_delete<T{}>()` will be used.
 */
template <class T, auto TDeleter = nullptr>
struct shared_ptr : std::shared_ptr<T> {
  explicit shared_ptr() = delete;
  explicit shared_ptr(nullptr_t) : std::shared_ptr<T>(nullptr) {
  }
  explicit shared_ptr(T* ptr)
    : std::shared_ptr<T>(ptr, detail::deleter_type_t<T, TDeleter> {}) {
  }
};
}// namespace FredEmmott::Memory