// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <cstdint>

#include "Point.hpp"
#include "Size.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkRect.h>
#endif

namespace FredEmmott::GUI {

template <class T>
  requires(std::integral<T> || std::floating_point<T>)
struct BasicRect {
  BasicPoint<T> mTopLeft;
  BasicSize<T> mSize;

  constexpr bool operator==(const BasicRect&) const noexcept = default;

  constexpr T GetWidth() const noexcept {
    return mSize.mWidth;
  }

  constexpr T GetHeight() const noexcept {
    return mSize.mHeight;
  }

  constexpr T GetLeft() const noexcept {
    return mTopLeft.mX;
  }

  constexpr T GetTop() const noexcept {
    return mTopLeft.mY;
  }

  constexpr T GetRight() const noexcept {
    return mTopLeft.mX + mSize.mWidth;
  }

  constexpr T GetBottom() const noexcept {
    return mTopLeft.mY + mSize.mHeight;
  }

  constexpr void Inset(T dx, T dy) noexcept {
    mTopLeft.mX += dx;
    mTopLeft.mY += dy;
    mSize.mWidth -= dx * 2;
    mSize.mHeight -= dy * 2;
  }

  template <class Self>
  constexpr Self WithInset(this const Self& self, T dx, T dy) noexcept {
    Self result = self;
    result.Inset(dx, dy);
    return result;
  }

#ifdef FUI_ENABLE_SKIA
  constexpr operator SkRect() const noexcept
    requires std::same_as<float, T>
  {
    return SkRect::MakeXYWH(
      mTopLeft.mX, mTopLeft.mY, mSize.mWidth, mSize.mHeight);
  }
#endif
};

using Rect = BasicRect<float>;

}// namespace FredEmmott::GUI