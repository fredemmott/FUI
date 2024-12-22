// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <functional>
#include <memory>

namespace FredEmmott::Memory::extensions {

template<class T>
struct deleter : std::default_delete<T> {};

}

namespace FredEmmott::Memory::memory_detail {
template <class T, void (*TCallback)(T*)>
struct callback_delete {
  void operator()(T* ptr) {
    std::invoke(TCallback, ptr);
  }
};

template <class T, auto TDeleter>
struct deleter_type {
  using type = callback_delete<T, TDeleter>;
};

template <class T>
struct deleter_type<T, nullptr> {
  using type = ::FredEmmott::Memory::extensions::deleter<T>;
};

template <class T, auto TDeleter>
using deleter_type_t = typename deleter_type<T, TDeleter>::type;
}// namespace FredEmmott::Memory::memory_detail
