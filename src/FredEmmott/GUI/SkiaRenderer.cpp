// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SkiaRenderer.hpp"

#include <FredEmmott/GUI/detail/renderer_detail.hpp>

#include "assert.hpp"

namespace FredEmmott::GUI {

SkiaRenderer::SkiaRenderer(SkCanvas* canvas) : mCanvas(canvas) {
  FUI_ASSERT(canvas != nullptr);
  FUI_ASSERT(
    renderer_detail::GetRenderAPI() == renderer_detail::RenderAPI::Skia);
}

SkiaRenderer::~SkiaRenderer() {
#ifndef NDEBUG
  FUI_ASSERT(mStackDepth == 0);
#endif
}

void SkiaRenderer::PushLayer(const float alpha) {
  if (std::abs(1.0f - alpha) < std::numeric_limits<float>::epsilon()) {
    mCanvas->save();
  } else {
    mCanvas->saveLayerAlpha(nullptr, alpha);
  }
#ifndef NDEBUG
  ++mStackDepth;
#endif
}

void SkiaRenderer::PopLayer() {
  mCanvas->restore();
#ifndef NDEBUG
  --mStackDepth;
#endif
}

void SkiaRenderer::Clear(const Color& color) {
  mCanvas->clear(color.as<SkColor>());
}

void SkiaRenderer::PushClipRect(const Rect& rect) {
  mCanvas->save();
  mCanvas->clipRect(rect);
#ifndef NDEBUG
  ++mStackDepth;
#endif
}

void SkiaRenderer::PopClipRect() {
  mCanvas->restore();
#ifndef NDEBUG
  --mStackDepth;
#endif
}

void SkiaRenderer::Scale(const float x, const float y) {
  mCanvas->scale(x, y);
}

void SkiaRenderer::Translate(const Point& point) {
  mCanvas->translate(point.mX, point.mY);
}

void SkiaRenderer::FillRect(const Brush& brush, const Rect& rect) {
  auto paint = brush.GetSkiaPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mCanvas->drawRect(rect, paint);
}

void SkiaRenderer::StrokeRect(
  const Brush& brush,
  const Rect& rect,
  float thickness) {
  auto paint = brush.GetSkiaPaint(rect);
  paint.setStyle(SkPaint::Style::kStroke_Style);
  paint.setStrokeWidth(thickness);
  mCanvas->drawRect(rect, paint);
}

void SkiaRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius) {
  auto paint = brush.GetSkiaPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  paint.setAntiAlias(true);
  mCanvas->drawRoundRect(rect, radius, radius, paint);
}

void SkiaRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius,
  float thickness) {
  auto paint = brush.GetSkiaPaint(rect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(thickness);
  paint.setAntiAlias(true);
  mCanvas->drawRoundRect(rect, radius, radius, paint);
}

void SkiaRenderer::DrawText(
  const Brush& brush,
  const Rect& brushRect,
  const Font& font,
  const std::string_view text,
  const Point& baseline) {
  auto paint = brush.GetSkiaPaint(brushRect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mCanvas->drawString(
    SkString {text}, baseline.mX, baseline.mY, font.as<SkFont>(), paint);
}

}// namespace FredEmmott::GUI