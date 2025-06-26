// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <d2d1_1.h>
#include <dwrite.h>

#include <stack>

#include "Renderer.hpp"

namespace FredEmmott::GUI {

class Direct2DRenderer final : public Renderer {
 public:
  Direct2DRenderer() = delete;
  explicit Direct2DRenderer(ID2D1DeviceContext* deviceContext);
  ~Direct2DRenderer() override;

  // State management
  void PushLayer(float alpha = 1.f) override;
  void PopLayer() override;

  // Basic drawing operations
  void Clear(const Color& color) override;
  void PushClipRect(const Rect& rect) override;
  void PopClipRect() override;

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

 private:
  struct Direct2DLayer : std::monostate {};
  using StateStackFrame = std::variant<Direct2DLayer, D2D1_MATRIX_3X2_F>;
  std::stack<StateStackFrame> mStateStack;

  ID2D1DeviceContext* mDeviceContext = nullptr;
};

}// namespace FredEmmott::GUI
