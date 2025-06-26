// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <wil/com.h>

#include "Brush.hpp"
#include "Color.hpp"
#include "Font.hpp"
#include "Point.hpp"
#include "Rect.hpp"

namespace FredEmmott::GUI {

class Renderer {
 public:
  virtual ~Renderer() = default;

  /// Push the clipping region and transform
  virtual void PushLayer(float alpha = 1.0f) = 0;
  virtual void PopLayer() = 0;
  auto ScopedLayer(float alpha = 1.0f) {
    PushLayer(alpha);
    return wil::scope_exit([this] { PopLayer(); });
  }

  virtual void Clear(const Color&) = 0;

  virtual void PushClipRect(const Rect&) = 0;
  virtual void PopClipRect() = 0;
  auto ScopedClipRect(const Rect& rect) {
    PushClipRect(rect);
    return wil::scope_exit([this] { PopClipRect(); });
  }

  virtual void Scale(float x, float y) = 0;

  void Scale(float scale) {
    this->Scale(scale, scale);
  }

  virtual void Translate(const Point&) = 0;
  void Translate(float x, float y) {
    this->Translate({x, y});
  }

  virtual void FillRect(const Brush& brush, const Rect& rect) = 0;
  virtual void
  StrokeRect(const Brush& brush, const Rect& rect, float thickness = 0)
    = 0;

  virtual void
  FillRoundedRect(const Brush& brush, const Rect& rect, float radius)
    = 0;
  virtual void StrokeRoundedRect(
    const Brush& brush,
    const Rect& rect,
    float radius,
    float thickness = 0)
    = 0;

  virtual void DrawText(
    const Brush& brush,
    const Rect& brushRect,
    const Font& font,
    std::string_view text,
    const Point& baseline)
    = 0;
};

}// namespace FredEmmott::GUI