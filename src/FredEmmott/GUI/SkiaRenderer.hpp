// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <FredEmmott/GUI/config.hpp>

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
  void FillRoundedRect(
    const Brush& brush,
    const Rect& rect,
    float topLeftRadius,
    float topRightRadius,
    float bottomRightRadius,
    float bottomLeftRadius) override;
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

inline SkiaRenderer* skia_renderer_cast(Renderer* renderer) noexcept {
  if constexpr (Config::HaveSingleBackend) {
    static_assert(Config::HaveSkia);
    return static_cast<SkiaRenderer*>(renderer);
  } else {
    return dynamic_cast<SkiaRenderer*>(renderer);
  }
}

inline SkCanvas* skia_canvas_cast(Renderer* renderer) noexcept {
  const auto skiaRenderer = skia_renderer_cast(renderer);
  if constexpr (Config::HaveSingleBackend) {
    return skiaRenderer->GetSkCanvas();
  } else {
    return skiaRenderer ? skiaRenderer->GetSkCanvas() : nullptr;
  }
}

}// namespace FredEmmott::GUI