// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <bit>
#include <format>
#include <source_location>
#include <string>

namespace FredEmmott::GUI::Immediate {

template <class T>
concept hashable = requires(T v) {
  { std::hash<T> {}(v) } -> std::convertible_to<std::size_t>;
};

class ID final {
 public:
  using value_type = uint64_t;
  ID() = delete;

  template <size_t N>
  explicit constexpr ID(const char (&id)[N])
    : ID(std::string_view {id, N - 1}) {}

  explicit constexpr ID(const std::convertible_to<value_type> auto id)
    : mID(id) {}

  template <hashable T>
    requires(!std::convertible_to<T, value_type>)
  explicit constexpr ID(const T& id) : mID(std::hash<T> {}(id)) {}

  explicit constexpr ID(const std::source_location& location);

  explicit constexpr ID(const std::string_view string) : mID(Hash(string)) {}

  template <class... TArgs>
    requires(sizeof...(TArgs) > 0)
  explicit
#ifdef __cpp_lib_constexpr_format// P3391
    constexpr
#endif
    ID(std::format_string<TArgs...> fmt, TArgs&&... args) {
    mID = Hash(std::format(fmt, std::forward<TArgs>(args)...));
  }

  [[nodiscard]]
  constexpr value_type GetValue() const noexcept {
    return mID;
  }

 private:
  value_type mID {};
  static_assert(sizeof(value_type) == 8);
  static constexpr value_type FNVPrime = 0x00000100000001b3;
  static constexpr value_type FNVOffsetBasis = 0xcbf29ce484222325;

  static constexpr value_type HashStep(
    const value_type hash,
    const uint8_t byte) {
    return (hash ^ byte) * FNVPrime;
  }

  template <std::integral T>
    requires(!std::same_as<uint8_t, T>)
  constexpr value_type static HashMixin(value_type hash, const T value) {
    for (auto&& byte: std::bit_cast<std::array<uint8_t, sizeof(T)>>(value)) {
      hash = HashStep(hash, byte);
    }
    return hash;
  }

  /// FNV-1a
  static constexpr value_type Hash(const std::string_view str) {
    value_type hash = FNVOffsetBasis;
    for (const auto byte: str) {
      hash = HashStep(hash, static_cast<uint8_t>(byte));
    }
    return hash;
  }
};

constexpr ID::ID(const std::source_location& location) {
  mID = Hash(location.file_name());
  mID = HashMixin(mID, location.line());
  mID = HashMixin(mID, location.column());
}

};// namespace FredEmmott::GUI::Immediate