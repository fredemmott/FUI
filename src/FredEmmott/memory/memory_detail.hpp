// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <functional>
#include <memory>

namespace FredEmmott::Memory::extensions {

template <class T>
struct deleter {
  deleter() = delete;
};

}// namespace FredEmmott::Memory::extensions

namespace FredEmmott::Memory::memory_detail {
template <class T, std::invocable<T*> auto TCallback>
struct callback_delete {
  void operator()(T* ptr) {
    std::invoke(TCallback, ptr);
  }
};

template <class T, auto TDeleter>
struct deleter_type;

template <class T>
struct deleter_type<T, nullptr> {
  using type = std::default_delete<T>;
};

// AIUI we *should* be able to put the requirement in to the template <>,
// but while Clang is fine with it, MSVC 19.44.35214 (VS 17.14.11) allows
// the syntax, but fails to substitute it where appropriate
template <class T, auto TDeleter>
  requires std::invocable<decltype(TDeleter), T*>
struct deleter_type<T, TDeleter> {
  using type = callback_delete<T, TDeleter>;
};

template <class T>
  requires std::invocable<extensions::deleter<T>, T*>
struct deleter_type<T, nullptr> {
  using type = ::FredEmmott::Memory::extensions::deleter<T>;
};

template <class T, auto TDeleter>
using deleter_type_t = typename deleter_type<T, TDeleter>::type;

}// namespace FredEmmott::Memory::memory_detail
