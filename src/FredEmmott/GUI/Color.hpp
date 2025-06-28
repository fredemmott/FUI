// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <source_location>
#include <stdexcept>
#include <variant>

#include "StaticTheme/Resource.hpp"
#include "SystemTheme.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkColor.h>
#endif

#ifdef FUI_ENABLE_DIRECT2D
#include <d2d1.h>
#endif

namespace FredEmmott::GUI {

template <class T>
concept native_color =
#ifdef FUI_ENABLE_SKIA
  std::same_as<T, SkColor> ||
#endif
#ifdef FUI_ENABLE_DIRECT2D
  std::same_as<T, D2D1_COLOR_F> ||
#endif
  false;

class Color final {
  using StaticThemeColor = const StaticTheme::Resource<Color>*;

 public:
  struct Constant final {
    static constexpr Constant
    FromRGBA128F(const float r, const float g, const float b, const float a) {
      Constant ret {};
#ifdef FUI_ENABLE_SKIA
      ret.mSkia = SkColorSetARGB(
        static_cast<uint8_t>(std::round(a * 255)),
        static_cast<uint8_t>(std::round(r * 255)),
        static_cast<uint8_t>(std::round(g * 255)),
        static_cast<uint8_t>(std::round(b * 255)));
#endif
#ifdef FUI_ENABLE_DIRECT2D
      ret.mD2D = {r, g, b, a};
#endif
      return ret;
    }

    static constexpr Constant FromRGBA32(
      const uint8_t r,
      const uint8_t g,
      const uint8_t b,
      const uint8_t a) {
      Constant ret {};
#ifdef FUI_ENABLE_SKIA
      ret.mSkia = SkColorSetARGB(a, r, g, b);
#endif
#ifdef FUI_ENABLE_DIRECT2D
      ret.mD2D = {
        .r = static_cast<float>(r) / 255.0f,
        .g = static_cast<float>(g) / 255.0f,
        .b = static_cast<float>(b) / 255.0f,
        .a = static_cast<float>(a) / 255.0f,
      };
#endif
      return ret;
    }

    static constexpr Constant FromBGRA32(const uint32_t bgra) {
      const auto [b, g, r, a] = std::bit_cast<std::array<uint8_t, 4>>(bgra);
      return FromRGBA32(r, g, b, a);
    }

    static constexpr Constant FromRGBA32(const uint32_t rgba) {
      const auto [r, g, b, a] = std::bit_cast<std::array<uint8_t, 4>>(rgba);
      return FromRGBA32(r, g, b, a);
    }

    static constexpr Constant FromARGB32(
      const uint8_t a,
      const uint8_t r,
      const uint8_t g,
      const uint8_t b) {
      return FromRGBA32(r, g, b, a);
    }

    [[nodiscard]] constexpr Constant WithAlphaMultipliedBy(
      const float mult) const noexcept {
      Constant ret = *this;
#ifdef FUI_ENABLE_SKIA
      ret.mSkia = SkColorSetA(
        mSkia, static_cast<U8CPU>(std::round(SkColorGetA(mSkia) * mult)));
#endif
#ifdef FUI_ENABLE_DIRECT2D
      ret.mD2D.a *= std::clamp(mult, 0.0f, 1.0f);
#endif
      return ret;
    }

    constexpr auto GetRGBAFTuple() const noexcept {
#ifdef FUI_ENABLE_DIRECT2D
      return std::tuple {mD2D.r, mD2D.g, mD2D.b, mD2D.a};
#elifdef FUI_ENABLE_SKIA
      return std::tuple {
        SkColorGetR(mSkia) / 255.0,
        SkColorGetG(mSkia) / 255.0,
        SkColorGetB(mSkia) / 255.0,
        SkColorGetA(mSkia) / 255.0,
      };
#endif
    }

    bool operator==(const Constant& other) const noexcept {
#ifdef FUI_ENABLE_SKIA
      return mSkia == other.mSkia;
#elifdef FUI_ENABLE_DIRECT2D
      return memcmp(&mD2D, &other.mD2D, sizeof(mD2D)) == 0;
#else
      static_assert(false, "No backends enabled for Colors")
#endif
    }

    template <native_color T>
    constexpr T as() const noexcept {
#ifdef FUI_ENABLE_SKIA
      if constexpr (std::same_as<T, SkColor>) {
        return mSkia;
      }
#endif
#ifdef FUI_ENABLE_DIRECT2D
      if constexpr (std::same_as<T, D2D1_COLOR_F>) {
        return mD2D;
      }
#endif
      std::unreachable();
    }

   private:
#ifdef FUI_ENABLE_SKIA
    SkColor mSkia {};
#endif
#ifdef FUI_ENABLE_DIRECT2D
    D2D1_COLOR_F mD2D {};
#endif
  };
  Color() = delete;
  constexpr Color(const Constant& color) : mVariant(color) {}
  constexpr Color(StaticThemeColor color) : mVariant(color) {
    if (!color) [[unlikely]] {
      throw std::logic_error("Static resource colors must be a valid pointer");
    }
  }
  constexpr Color(nullptr_t) = delete;
  constexpr Color(SystemTheme::ColorType u) : mVariant(u) {}

  constexpr bool operator==(const Color& other) const noexcept = default;

  template <StaticTheme::Theme TTheme>
  constexpr Constant ResolveForStaticTheme() const {
    if (const auto it = get_if<Constant>(&mVariant)) {
      return *it;
    }
    if (const auto it = get_if<StaticThemeColor>(&mVariant)) {
      return (*it)->Resolve(TTheme).Resolve();
    }
    throw std::bad_variant_access {};
  }

  template <native_color T>
  T as() const noexcept {
    return Resolve().as<T>();
  }

  constexpr auto GetRGBAFTuple() const noexcept {
    return Resolve().GetRGBAFTuple();
  }

 private:
  std::variant<Color::Constant, StaticThemeColor, SystemTheme::ColorType>
    mVariant;

  constexpr Constant Resolve() const noexcept {
    if (const auto it = get_if<Constant>(&mVariant)) {
      return *it;
    }
    if (const auto it = get_if<StaticThemeColor>(&mVariant)) {
      return (*it)->Resolve().Resolve();
    }
    if (const auto it = get_if<SystemTheme::ColorType>(&mVariant)) {
      return SystemTheme::Resolve(*it).Resolve();
    }
    std::unreachable();
  }
};

}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::Colors {
constexpr auto Transparent = Color::Constant::FromRGBA32(0, 0, 0, 0);

constexpr auto Black = Color::Constant::FromRGBA32(0x00, 0x00, 0x00, 0xff);
constexpr auto LightGray = Color::Constant::FromRGBA32(0xcc, 0xcc, 0xcc, 0xff);
constexpr auto White = Color::Constant::FromRGBA32(0xff, 0xff, 0xff, 0xff);

constexpr auto Red = Color::Constant::FromRGBA32(0xff, 0x00, 0x00, 0xff);
constexpr auto Green = Color::Constant::FromRGBA32(0x00, 0xff, 0x00, 0xff);
constexpr auto Blue = Color::Constant::FromRGBA32(0x00, 0x00, 0xff, 0xff);
}// namespace FredEmmott::GUI::Colors