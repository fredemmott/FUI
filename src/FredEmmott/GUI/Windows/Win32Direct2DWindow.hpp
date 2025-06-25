// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <wil/com.h>

#include "Win32Window.hpp"

namespace FredEmmott::GUI {
class Direct2DRenderer;

class Win32Direct2DWindow final : public Win32Window {
 public:
  Win32Direct2DWindow(
    HINSTANCE instance,
    UINT showCommand,
    const Options& options = {});
  ~Win32Direct2DWindow() override;

 protected:
  void InitializeGraphicsAPI() override;
  IUnknown* GetDirectCompositionTargetDevice() const override;
  void CreateRenderTargets() override;
  void CleanupFrameContexts() override;
  std::unique_ptr<Win32Window> CreatePopup(
    HINSTANCE instance,
    int showCommand,
    const Options& options) const override;

 private:
  struct SharedResources;
  std::shared_ptr<SharedResources> mSharedResources;
  static std::weak_ptr<SharedResources> gSharedResources;

  wil::com_ptr<ID3D11Device> mD3DDevice;
  wil::com_ptr<ID3D11DeviceContext> mD3DDeviceContext;
  wil::com_ptr<ID2D1Factory3> mD2DFactory;
  wil::com_ptr<ID2D1Device2> mD2DDevice;
  wil::com_ptr<ID2D1DeviceContext2> mD2DDeviceContext;
  wil::com_ptr<IDWriteFactory> mDWriteFactory;

  struct FrameContext {
    wil::com_ptr<ID2D1Bitmap1> mD2DTargetBitmap;
  };

  // We're ultimately backed by D3D11, which exposes a single magical buffer
  // which automatically rotates to the front of the swapchain
  FrameContext mFrame {};

  void InitializeD3D();
  void InitializeDirect2D();

  std::unique_ptr<BasicFramePainter> GetFramePainter(
    uint8_t frameIndex) override;
  class FramePainter;
  friend class FramePainter;

  void BeforePaintFrame(uint8_t frameIndex);
  void AfterPaintFrame(uint8_t frameIndex);
};
}// namespace FredEmmott::GUI
