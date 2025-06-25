// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Direct2DRenderer.hpp"

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Font.hpp>
#include <FredEmmott/GUI/Rect.hpp>

namespace FredEmmott::GUI {

Direct2DRenderer::Direct2DRenderer(ID2D1DeviceContext* deviceContext)
  : mDeviceContext(deviceContext) {}

void Direct2DRenderer::PushLayer(float alpha) {}
void Direct2DRenderer::PopLayer() {}
void Direct2DRenderer::ClipTo(const Rect& rect) {}
void Direct2DRenderer::Scale(float x, float y) {}
void Direct2DRenderer::Translate(const Point& point) {}
void Direct2DRenderer::FillRect(const Brush& brush, const Rect& rect) {}
void Direct2DRenderer::StrokeRect(
  const Brush& brush,
  const Rect& rect,
  float thickness) {}
void Direct2DRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius) {}
void Direct2DRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius,
  float thickness) {}
void Direct2DRenderer::DrawText(
  const Brush& brush,
  const Rect& brushRect,
  const Font& font,
  std::string_view text,
  const Point& baseline) {}

void Direct2DRenderer::Clear(const Color& color) {
  mDeviceContext->Clear(color.as<D2D1_COLOR_F>());
}

}// namespace FredEmmott::GUI
