// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <d2d1_1.h>
#include <d3d11_4.h>
#include <dwrite.h>

#include <FredEmmott/GUI/config.hpp>
#include <stack>

#include "Renderer.hpp"
#include "Size.hpp"

namespace FredEmmott::GUI {

class Direct2DRenderer final : public Renderer {
 public:
  Direct2DRenderer() = delete;
  explicit Direct2DRenderer(
    ID3D11Device5* device,
    ID2D1DeviceContext* deviceContext);
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

  void DrawLine(
    const Brush& brush,
    const Point& start,
    const Point& end,
    float thickness) override;

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

  [[nodiscard]]
  std::unique_ptr<ImportedTexture> ImportTexture(
    ImportedTexture::HandleKind,
    HANDLE) const;

  [[nodiscard]]
  std::unique_ptr<ImportedFence> ImportFence(HANDLE) const;

  void DrawTexture(
    const Rect& sourceRect,
    const Rect& destRect,
    ImportedTexture* texture,
    ImportedFence* fence,
    uint64_t fenceValue);

 private:
  struct StateStackFrame {
    D2D1_MATRIX_3X2_F mTransform {};
    bool mHaveNativeLayer {false};
  };
  ID3D11Device5* mD3DDevice = nullptr;
  wil::com_ptr<ID3D11DeviceContext4> mD3DDeviceContext;
  ID2D1DeviceContext* mDeviceContext = nullptr;

  std::stack<StateStackFrame> mStateStack;

  void PostTransform(const D2D1_MATRIX_3X2_F&);

  friend ID2D1DeviceContext* direct2d_device_context_cast(
    Renderer* renderer) noexcept;
};

inline Direct2DRenderer* direct2d_renderer_cast(Renderer* renderer) noexcept {
  if constexpr (Config::HaveSingleBackend) {
    static_assert(Config::HaveDirect2D);
    return static_cast<Direct2DRenderer*>(renderer);
  } else {
    return dynamic_cast<Direct2DRenderer*>(renderer);
  }
}

inline ID2D1DeviceContext* direct2d_device_context_cast(
  Renderer* renderer) noexcept {
  const auto direct2dRenderer = direct2d_renderer_cast(renderer);
  if constexpr (Config::HaveSingleBackend) {
    return direct2dRenderer->mDeviceContext;
  } else {
    return direct2dRenderer ? direct2dRenderer->mDeviceContext : nullptr;
  }
}

}// namespace FredEmmott::GUI
