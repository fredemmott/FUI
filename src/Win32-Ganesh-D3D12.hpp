// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <skia/gpu/GrDirectContext.h>
#include <wil/com.h>
#include <wil/resource.h>

#include <optional>

class HelloSkiaWindow final {
 public:
  HelloSkiaWindow() = delete;
  HelloSkiaWindow(const HelloSkiaWindow&) = delete;
  HelloSkiaWindow(HelloSkiaWindow&&) = delete;
  HelloSkiaWindow& operator=(const HelloSkiaWindow&) = delete;
  HelloSkiaWindow& operator=(HelloSkiaWindow&&) = delete;

  explicit HelloSkiaWindow(HINSTANCE instance);
  ~HelloSkiaWindow();

  [[nodiscard]] HWND GetHWND() const noexcept;
  [[nodiscard]] int Run() noexcept;

 private:
  static constexpr UINT SwapChainLength = 3;

  static HelloSkiaWindow* gInstance;

  wil::unique_hwnd mHwnd;
  std::optional<int> mExitCode;

  struct PixelSize {
    UINT mWidth {};
    UINT mHeight {};
  };
  PixelSize mWindowSize;
  std::optional<PixelSize> mPendingResize;

  wil::com_ptr<IDXGIAdapter1> mDXGIAdapter;
  wil::com_ptr<ID3D12Device> mD3DDevice;
  wil::com_ptr<ID3D12CommandQueue> mD3DCommandQueue;
  wil::com_ptr<ID3D12GraphicsCommandList> mD3DCommandList;
  wil::com_ptr<ID3D12DescriptorHeap> mD3DRTVHeap;
  wil::com_ptr<ID3D12DescriptorHeap> mD3DSRVHeap;
  wil::com_ptr<IDXGISwapChain1> mSwapChain;

  wil::com_ptr<ID3D12Fence> mD3DFence;
  wil::unique_handle mFenceEvent {CreateEventW(nullptr, FALSE, FALSE, nullptr)};
  uint64_t mFenceValue = 0;

  sk_sp<GrDirectContext> mSkiaContext;

  struct FrameContext {
    wil::com_ptr<ID3D12CommandAllocator> mCommandAllocator;
    wil::com_ptr<ID3D12Resource> mRenderTarget;
    D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetView {};
    sk_sp<SkSurface> mSkSurface;

    uint64_t mFenceValue {};
  };
  uint64_t mFrameCounter {};
  std::array<FrameContext, SwapChainLength> mFrames;

  void CreateRenderTargets();
  void CleanupFrameContexts();

  static LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
};
