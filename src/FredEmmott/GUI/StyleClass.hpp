// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <bit>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace FredEmmott::GUI {
struct NegatedStyleClass;

class StyleClass {
  friend struct std::hash<StyleClass>;

 public:
  StyleClass() = delete;
  StyleClass(const StyleClass&) = default;
  StyleClass(StyleClass&&) = default;
  StyleClass& operator=(const StyleClass&) = default;
  StyleClass& operator=(StyleClass&&) = default;

  static StyleClass Make(std::string_view name);

  bool operator==(const StyleClass&) const noexcept = default;

  inline NegatedStyleClass operator!() const noexcept;

  uintptr_t AsCacheKey() const noexcept {
    return std::bit_cast<uintptr_t>(mID.data());
  }

  std::string_view GetName() const noexcept {
    return mID;
  }

 private:
  explicit StyleClass(std::string_view id) : mID(id) {}
  std::string_view mID {};
};
using StyleClasses = std::unordered_set<StyleClass>;
StyleClasses operator+(const StyleClasses&, StyleClass);
StyleClasses& operator+=(StyleClasses&, StyleClass);

struct NegatedStyleClass {
  StyleClass mStyleClass;
  bool operator==(const NegatedStyleClass&) const noexcept = default;
  StyleClass operator!() const noexcept {
    return mStyleClass;
  }
};

NegatedStyleClass StyleClass::operator!() const noexcept {
  return NegatedStyleClass {*this};
}

template <std::size_t N>
class LiteralStyleClass {
 public:
  LiteralStyleClass() = delete;
  constexpr explicit LiteralStyleClass(const char (&name)[N + 1]) noexcept {
    for (std::size_t i = 0; i < N; ++i) {
      mName[i] = name[i];
    }
  }

  StyleClass Get() const noexcept {
    if (!mCache.has_value()) {
      const std::string_view sv {mName, N};
      mCache = StyleClass::Make(sv);
    }
    return mCache.value();
  }

  constexpr operator FredEmmott::GUI::StyleClass() const noexcept {
    return Get();
  }

  constexpr auto operator*() const noexcept {
    return Get();
  }

 private:
  char mName[N] {};
  mutable std::optional<StyleClass> mCache;
};
template <std::size_t N>
LiteralStyleClass(const char (&)[N]) -> LiteralStyleClass<N - 1>;

}// namespace FredEmmott::GUI

template <>
struct std::hash<FredEmmott::GUI::StyleClass> {
  std::size_t operator()(const FredEmmott::GUI::StyleClass& c) const noexcept {
    return std::hash<uintptr_t> {}(std::bit_cast<uintptr_t>(c.mID.data()));
  }
};

template <>
struct std::hash<FredEmmott::GUI::NegatedStyleClass> {
  std::size_t operator()(
    const FredEmmott::GUI::NegatedStyleClass& c) const noexcept {
    return ~std::hash<FredEmmott::GUI::StyleClass> {}(c.mStyleClass);
  }
};
