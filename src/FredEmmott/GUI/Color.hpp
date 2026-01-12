// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <felly/overload.hpp>
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

namespace detail {

template <class T>
struct is_native_color_t : std::false_type {};

#ifdef FUI_ENABLE_SKIA
template <>
struct is_native_color_t<SkColor> : std::true_type {};
#endif

#ifdef FUI_ENABLE_DIRECT2D
template <>
struct is_native_color_t<D2D1_COLOR_F> : std::true_type {};
#endif

}// namespace detail

template <class T>
concept native_color = detail::is_native_color_t<T>::value;

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
#else
      const auto r8 = std::clamp<uint8_t>(std::roundl(r * 0xff), 0, 0xff);
      const auto g8 = std::clamp<uint8_t>(std::roundl(g * 0xff), 0, 0xff);
      const auto b8 = std::clamp<uint8_t>(std::roundl(b * 0xff), 0, 0xff);
      const auto a8 = std::clamp<uint8_t>(std::roundl(a * 0xff), 0, 0xff);
      std::bit_cast<std::array<uint8_t, 4>>(ret.mBGRA32) = {b8, g8, r8, a8};
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
#else
      std::bit_cast<std::array<uint8_t, 4>>(ret.mBGRA32) = {b, g, r, a};
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

    constexpr std::tuple<float, float, float, float> GetRGBAFTuple()
      const noexcept {
#ifdef FUI_ENABLE_DIRECT2D
      return std::tuple {mD2D.r, mD2D.g, mD2D.b, mD2D.a};
#elifdef FUI_ENABLE_SKIA
      return std::tuple {
        SkColorGetR(mSkia) / 255.0f,
        SkColorGetG(mSkia) / 255.0f,
        SkColorGetB(mSkia) / 255.0f,
        SkColorGetA(mSkia) / 255.0f,
      };
#endif
    }

    [[nodiscard]] constexpr Constant WithAlphaMultipliedBy(
      const float mult) const noexcept {
      if (mult < std::numeric_limits<float>::epsilon()) {
        return FromRGBA128F(0, 0, 0, 0);
      }
      if (mult > 1.f - std::numeric_limits<float>::epsilon()) {
        return *this;
      }

      auto [r, g, b, a] = GetRGBAFTuple();
      a = std::clamp(a * mult, 0.0f, 1.0f);
      return FromRGBA128F(r, g, b, a);
    }

    constexpr bool operator==(const Constant& other) const noexcept {
#ifdef FUI_ENABLE_SKIA
      return mSkia == other.mSkia;
#else
      return mBGRA32 == other.mBGRA32;
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
    static_assert(std::endian::native == std::endian::little);
    // Equivalently...
    static_assert(SkColorSetARGB(0, 0xff, 0, 0) == 0x00ff0000);
#else
    // Used for optimizations, especially equality when the native type has
    // floats, to avoid the need to do 'almost equal' checks for every component
    uint32_t mBGRA32 {};
#endif
#ifdef FUI_ENABLE_DIRECT2D
    D2D1_COLOR_F mD2D {};
#endif
  };
  Color() = delete;
  constexpr Color(const Constant& color) : mVariant(color) {}
  constexpr Color(nullptr_t) = delete;
  constexpr Color(SystemTheme::ColorType u) : mVariant(u) {}

  constexpr bool operator==(const Color& other) const noexcept {
    return Resolve() == other.Resolve();
  }

  template <StaticTheme::Theme TTheme>
  constexpr Constant ResolveForStaticTheme() const {
    return std::visit(
      felly::overload {
        [](const Constant& v) -> Constant { return v; },
        [](const StaticThemeColor c) -> Constant {
          return c->Resolve(TTheme)->Resolve();
        },
        [](const SystemTheme::ColorType c) -> Constant {
          return SystemTheme::Resolve(c).ResolveForStaticTheme<TTheme>();
        },
      },
      mVariant);
  }

  template <native_color T>
  T as() const noexcept {
    return Resolve().as<T>();
  }

  constexpr auto GetRGBAFTuple() const noexcept {
    return Resolve().GetRGBAFTuple();
  }

  constexpr Color WithAlphaMultipliedBy(float alpha) const noexcept {
    auto dup = *this;
    dup.mAlpha *= std::clamp(alpha, 0.f, 1.f);
    return dup;
  }

 private:
  std::variant<Color::Constant, StaticThemeColor, SystemTheme::ColorType>
    mVariant;
  float mAlpha {1.f};

  constexpr Constant Resolve() const noexcept {
    if (const auto it = get_if<Constant>(&mVariant)) {
      return it->WithAlphaMultipliedBy(mAlpha);
    }
    if (const auto it = get_if<StaticThemeColor>(&mVariant)) {
      return (*it)->Resolve()->Resolve().WithAlphaMultipliedBy(mAlpha);
    }
    if (const auto it = get_if<SystemTheme::ColorType>(&mVariant)) {
      return SystemTheme::Resolve(*it).Resolve().WithAlphaMultipliedBy(mAlpha);
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