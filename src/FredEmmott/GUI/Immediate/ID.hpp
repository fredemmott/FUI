// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <source_location>
#include <string>

namespace FredEmmott::GUI::Immediate {

template <class T>
concept hashable = requires(T v) {
  { std::hash<T> {}(v) } -> std::convertible_to<std::size_t>;
};

class ID final {
 public:
  ID() = delete;

  template <hashable T>
  explicit constexpr ID(const T& id) : mID(std::hash<T> {}(id)) {}

  template <size_t N>
  explicit constexpr ID(const char (&id)[N])
    : ID(std::string_view {id, N - 1}) {}

  explicit constexpr ID(const std::size_t id) : mID(id) {}

  explicit constexpr ID(const std::source_location& location)
    : mID((location.line() << 24) | (location.column() & 0xff)) {}

  [[nodiscard]]
  constexpr std::size_t GetValue() const noexcept {
    return mID;
  }

 private:
  std::size_t mID;
};
};// namespace FredEmmott::GUI::Immediate