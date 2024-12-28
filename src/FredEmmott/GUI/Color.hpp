// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <variant>

#include "StaticTheme/Resource.hpp"
#include "SystemTheme.hpp"

namespace FredEmmott::GUI {
class Color;
}

namespace FredEmmott::GUI {

class Color final {
  using StaticThemeColor = const StaticTheme::Resource<Color>*;

 public:
  Color() = delete;
  constexpr Color(SkColor color) : mVariant(color) {
  }
  constexpr Color(StaticThemeColor color) : mVariant(color) {
    if (!color) [[unlikely]] {
      throw std::logic_error("Static resource colors must be a valid pointer");
    }
  }
  constexpr Color(SystemTheme::ColorType u) : mVariant(u) {
  }

  Color operator*(std::floating_point auto alpha) const noexcept {
    const auto self = this->Resolve();
    return SkColorSetA(self, alpha * SkColorGetA(self));
  }

  constexpr operator SkColor() const noexcept {
    return this->Resolve();
  }

  constexpr bool operator==(const Color& other) const noexcept = default;

  template <StaticTheme::Theme TTheme>
  constexpr SkColor ResolveForStaticTheme() const {
    if (const auto it = get_if<SkColor>(&mVariant)) {
      return *it;
    }
    if (const auto it = get_if<StaticThemeColor>(&mVariant)) {
      return (*it)->Resolve(TTheme);
    }
    throw std::bad_variant_access {};
  }

 private:
  std::variant<SkColor, StaticThemeColor, SystemTheme::ColorType> mVariant;

  constexpr SkColor Resolve() const noexcept {
    if (const auto it = get_if<SkColor>(&mVariant)) {
      return *it;
    }
    if (const auto it = get_if<StaticThemeColor>(&mVariant)) {
      return (*it)->Resolve();
    }
    if (const auto it = get_if<SystemTheme::ColorType>(&mVariant)) {
      return SystemTheme::Resolve(*it);
    }
    std::unreachable();
  }
};

}// namespace FredEmmott::GUI