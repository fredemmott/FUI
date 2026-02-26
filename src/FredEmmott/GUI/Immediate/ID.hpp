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
  ID() = delete;

  template <hashable T>
  explicit constexpr ID(const T& id) : mID(std::hash<T> {}(id)) {}

  template <size_t N>
  explicit constexpr ID(const char (&id)[N])
    : ID(std::string_view {id, N - 1}) {}

  explicit constexpr ID(const std::size_t id) : mID(id) {}

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
  constexpr std::size_t GetValue() const noexcept {
    return mID;
  }

 private:
  std::size_t mID {};
  constexpr std::size_t static HashStep(
    const std::size_t hash,
    const uint8_t byte) {
    return (hash ^ byte) * FNVPrime();
  }

  template <std::integral T>
    requires(!std::same_as<uint8_t, T>)
  constexpr std::size_t static HashMixin(std::size_t hash, const T value) {
    for (auto&& byte: std::bit_cast<std::array<uint8_t, sizeof(T)>>(value)) {
      hash = HashStep(hash, byte);
    }
    return hash;
  }

  // ReSharper disable once CppDFAConstantFunctionResult
  static constexpr std::size_t FNVPrime() {
    if constexpr (sizeof(std::size_t) == 8) {
      return 0x00000100000001b3;
    } else if constexpr (sizeof(std::size_t) == 4) {
      return 0x01000193;
    }
  }

  // ReSharper disable once CppDFAConstantFunctionResult
  static constexpr std::size_t FNVOffsetBasis() {
    if constexpr (sizeof(std::size_t) == 8) {
      return 0xcbf29ce484222325;
    } else if constexpr (sizeof(std::size_t) == 4) {
      return 0x811c9dc5;
    }
  }

  /// FNV-1a
  static constexpr std::size_t Hash(const std::string_view str) {
    std::size_t hash = FNVOffsetBasis();
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