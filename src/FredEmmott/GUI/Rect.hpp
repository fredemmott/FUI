// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <cstdint>

#include "Point.hpp"
#include "Size.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkRect.h>
#endif

#ifdef FUI_ENABLE_DIRECT2D
#include <D2d1.h>
#endif

namespace FredEmmott::GUI {

template <class T>
  requires(std::integral<T> || std::floating_point<T>)
struct BasicRect {
  BasicPoint<T> mTopLeft;
  BasicSize<T> mSize;

  constexpr BasicRect(const BasicSize<T>& size) : mSize(size) {}

  constexpr BasicRect(const BasicPoint<T>& topLeft, const BasicSize<T>& size)
    : mTopLeft(topLeft),
      mSize(size) {}

  constexpr BasicRect(
    const BasicPoint<T>& topLeft,
    const BasicPoint<T>& bottomRight)
    : mTopLeft(topLeft) {
    mSize = {
      bottomRight.mX - topLeft.mX,
      bottomRight.mY - topLeft.mY,
    };
  }

  constexpr bool operator==(const BasicRect&) const noexcept = default;

  constexpr BasicPoint<T> GetTopLeft() const noexcept {
    return mTopLeft;
  }

  constexpr BasicPoint<T> GetTopRight() const noexcept {
    return {mTopLeft.mX + mSize.mWidth, mTopLeft.mY};
  }

  constexpr BasicPoint<T> GetBottomRight() const noexcept {
    return mTopLeft + mSize;
  }

  constexpr BasicPoint<T> GetBottomLeft() const noexcept {
    return {mTopLeft.mX, mTopLeft.mY + mSize.mHeight};
  }

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

  constexpr void Inset(T left, T top, T right, T bottom) noexcept {
    mTopLeft.mX += left;
    mTopLeft.mY += top;
    mSize.mWidth -= left + right;
    mSize.mHeight -= top + bottom;
  }

  template <class Self>
  constexpr Self WithInset(this const Self& self, T dx, T dy) noexcept {
    Self result = self;
    result.Inset(dx, dy);
    return result;
  }

  template <class Self>
  constexpr Self
  WithInset(this const Self& self, T left, T top, T right, T bottom) noexcept {
    Self result = self;
    result.Inset(left, top, right, bottom);
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
#ifdef FUI_ENABLE_DIRECT2D
  constexpr operator D2D1_RECT_F() const noexcept
    requires std::same_as<float, T>
  {
    return {GetLeft(), GetTop(), GetRight(), GetBottom()};
  }
#endif
  constexpr operator RECT() const noexcept
    requires std::same_as<LONG, T>
  {
    return {GetLeft(), GetTop(), GetRight(), GetBottom()};
  }
};

using Rect = BasicRect<float>;

}// namespace FredEmmott::GUI