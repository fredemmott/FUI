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
  struct DeviceResources {
    ID3D11Device5* mD3DDevice {};
    ID3D11DeviceContext4* mD3DDeviceContext {};
    ID2D1Factory* mD2DFactory {};
    ID2D1DeviceContext* mD2DDeviceContext {};
    ID2D1StrokeStyle* mD2DStrokeStyleRoundCap {};
    ID2D1StrokeStyle* mD2DStrokeStyleSquareCap {};
  };
  Direct2DRenderer() = delete;
  explicit Direct2DRenderer(
    DWORD dpi,
    const DeviceResources& deviceResources,
    std::shared_ptr<GPUCompletionFlag> frameCompletionFlag);
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
  void Rotate(float degrees, const Point& center) override;

  // Rectangle drawing
  void FillRect(const Brush& brush, const Rect& rect) override;
  void StrokeRect(const Brush& brush, const Rect& rect, float thickness)
    override;

  void DrawLine(
    const Brush& brush,
    const Point& start,
    const Point& end,
    float thickness,
    StrokeCap) override;

  // Rounded rectangle drawing
  void FillRoundedRect(
    const Brush& brush,
    const Rect& rect,
    const CornerRadius&) override;
  void StrokeRoundedRect(
    const Brush& brush,
    const Rect& rect,
    const CornerRadius&,
    Edges edges,
    float thickness) override;

  void StrokeArc(
    const Brush& brush,
    const Rect& rect,
    float startAngle,
    float sweepAngle,
    float thickness,
    StrokeCap strokeCap) override;
  void StrokeEllipse(const Brush& brush, const Rect& rect, float thickness)
    override;

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
    HANDLE) const override;
  [[nodiscard]]
  std::unique_ptr<ImportedTexture> ImportSoftwareBitmap(
    const SoftwareBitmap& in) const override;

  [[nodiscard]]
  std::unique_ptr<ImportedFence> ImportFence(HANDLE) const override;

  void DrawTexture(
    const Rect& sourceRect,
    const Rect& destRect,
    ImportedTexture* texture,
    ImportedFence* fence,
    uint64_t fenceValue) override;

  std::shared_ptr<GPUCompletionFlag> GetGPUCompletionFlagForCurrentFrame()
    const override;

  // Not calling this 'GetNativeDevice' because there's both the D2D and D3D
  // ones
  auto GetD3DDevice() const {
    return mDeviceResources.mD3DDevice;
  }

  [[nodiscard]] uint64_t GetPhysicalLength(const uint64_t length) override {
    return (length * mDPI) / USER_DEFAULT_SCREEN_DPI;
  }

  [[nodiscard]] float GetPhysicalLength(const float length) override {
    return (length * mDPIScale);
  }

 private:
  struct StateStackFrame {
    D2D1_MATRIX_3X2_F mTransform {};
    bool mHaveNativeLayer {false};
  };
  DWORD mDPI {USER_DEFAULT_SCREEN_DPI};
  float mDPIScale {1.0f};

  DeviceResources mDeviceResources {};
  std::shared_ptr<GPUCompletionFlag> mFrameCompletionFlag;
  std::stack<StateStackFrame> mStateStack;

  void PostTransform(const D2D1_MATRIX_3X2_F&);

  friend ID2D1DeviceContext* direct2d_device_context_cast(
    Renderer* renderer) noexcept;

  [[nodiscard]]
  ID2D1StrokeStyle* GetStrokeStyle(StrokeCap) const;

  [[nodiscard]]
  wil::com_ptr<ID2D1PathGeometry> MakeRoundedRectPathGeometry(
    const Rect& rect,
    const CornerRadius& radii,
    Edges edges) const;
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
    return direct2dRenderer->mDeviceResources.mD2DDeviceContext;
  } else {
    return direct2dRenderer
      ? direct2dRenderer->mDeviceResources.mD2DDeviceContext
      : nullptr;
  }
}

}// namespace FredEmmott::GUI
