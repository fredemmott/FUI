// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include "Renderer.hpp"

namespace FredEmmott::GUI {

class SkiaRenderer final : public Renderer {
 public:
  SkiaRenderer() = delete;
  explicit SkiaRenderer(SkCanvas*);
  ~SkiaRenderer() override;

  // State management
  void PushLayer(float alpha) override;
  void PopLayer() override;

  // Basic drawing operations
  void Clear(const Color& color) override;
  void PushClipRect(const Rect& rect) override;
  void PopClipRect() override;

  void DrawLine(
    const Brush& brush,
    const Point& start,
    const Point& end,
    float thickness) override;

  // Transformations
  void Scale(float x, float y) override;
  void Translate(const Point& point) override;

  // Rectangle drawing
  void FillRect(const Brush& brush, const Rect& rect) override;
  void StrokeRect(const Brush& brush, const Rect& rect, float thickness)
    override;

  // Rounded rectangle drawing
  void FillRoundedRect(const Brush& brush, const Rect& rect, float radius)
    override;
  void StrokeRoundedRect(
    const Brush& brush,
    const Rect& rect,
    float radius,
    float thickness) override;

  // Text drawing
  void DrawText(
    const Brush& brush,
    const Rect& brushRect,
    const Font& font,
    std::string_view text,
    const Point& baseline) override;

  SkCanvas* GetSkCanvas() const noexcept {
    return mCanvas;
  }

 private:
  SkCanvas* mCanvas {nullptr};
#ifndef NDEBUG
  std::size_t mStackDepth {};
#endif
};

constexpr SkiaRenderer* skia_renderer_cast(Renderer* renderer) noexcept {
  return dynamic_cast<SkiaRenderer*>(renderer);
}

constexpr SkCanvas* skia_canvas_cast(Renderer* renderer) noexcept {
  const auto skiaRenderer = skia_renderer_cast(renderer);
  return skiaRenderer ? skiaRenderer->GetSkCanvas() : nullptr;
}

}// namespace FredEmmott::GUI