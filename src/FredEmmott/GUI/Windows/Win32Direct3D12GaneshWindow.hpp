// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <d3d12.h>
#include <skia/core/SkSurface.h>

#if __has_include(<skia/gpu/ganesh/GrDirectContext.h>)
#include <skia/gpu/ganesh/GrDirectContext.h>
#else
#include <skia/gpu/GrDirectContext.h>
#endif

#include "Win32Window.hpp"

namespace FredEmmott::GUI {
class Win32Direct3D12GaneshWindow final : public Win32Window {
 public:
  Win32Direct3D12GaneshWindow(
    HINSTANCE instance,
    UINT showCommand,
    const Options& options = {});
  ~Win32Direct3D12GaneshWindow() override;

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

  wil::com_ptr<IDXGIAdapter1> mDXGIAdapter;
  wil::com_ptr<ID3D12Device> mD3DDevice;
  wil::com_ptr<ID3D12CommandQueue> mD3DCommandQueue;
  wil::com_ptr<ID3D12DescriptorHeap> mD3DRTVHeap;
  wil::com_ptr<ID3D12DescriptorHeap> mD3DSRVHeap;

  wil::com_ptr<ID3D12Fence> mD3DFence;
  wil::unique_handle mFenceEvent {CreateEventW(nullptr, FALSE, FALSE, nullptr)};
  uint64_t mFenceValue = 0;

  sk_sp<GrDirectContext> mSkContext;

  struct FrameContext {
    wil::com_ptr<ID3D12Resource> mRenderTarget;
    D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetView {};
    sk_sp<SkSurface> mSkSurface;

    uint64_t mFenceValue {};
  };

  std::array<FrameContext, SwapChainLength> mFrames;

  void InitializeD3D();
  void InitializeSkia();

  std::unique_ptr<BasicFramePainter> GetFramePainter(
    uint8_t mFrameIndex) override;
  class FramePainter;
  friend class FramePainter;

  void BeforePaintFrame(uint8_t frameIndex);
  void AfterPaintFrame(uint8_t frameIndex);
};
}// namespace FredEmmott::GUI
