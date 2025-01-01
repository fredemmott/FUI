// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <Windows.h>
#include <core/SkCanvas.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <skia/gpu/GrDirectContext.h>
#include <wil/com.h>
#include <wil/resource.h>

#include <FredEmmott/GUI/Immediate/Root.hpp>
#include <expected>
#include <optional>

class HelloSkiaWindow final {
 public:
  HelloSkiaWindow() = delete;
  HelloSkiaWindow(const HelloSkiaWindow&) = delete;
  HelloSkiaWindow(HelloSkiaWindow&&) = delete;
  HelloSkiaWindow& operator=(const HelloSkiaWindow&) = delete;
  HelloSkiaWindow& operator=(HelloSkiaWindow&&) = delete;

  void InitializeSkia();
  explicit HelloSkiaWindow(HINSTANCE instance);
  ~HelloSkiaWindow();

  [[nodiscard]] HWND GetHWND() const noexcept;
  void RenderFUIContent();
  void ResizeIfNeeded();
  [[nodiscard]] int Run() noexcept;

  [[nodiscard]]
  std::expected<void, int> BeginFrame();
  void WaitFrame(
    unsigned int minFPS = 0,
    unsigned int maxFPS = std::numeric_limits<unsigned int>::max()) const;

 private:
  static constexpr UINT SwapChainLength = 3;

  static thread_local std::unordered_map<HWND, HelloSkiaWindow*> gInstances;

  wil::unique_hwnd mHwnd;
  std::optional<int> mExitCode;

  float mDPIScale = {1.0f};
  DWORD mDPI = USER_DEFAULT_SCREEN_DPI;
  SkISize mWindowSize {};
  std::optional<SkISize> mPendingResize;

  FredEmmott::GUI::Immediate::Root mFUIRoot;

  wil::com_ptr<IDXGIAdapter1> mDXGIAdapter;
  wil::com_ptr<ID3D12Device> mD3DDevice;
  wil::com_ptr<ID3D12CommandQueue> mD3DCommandQueue;
  wil::com_ptr<ID3D12DescriptorHeap> mD3DRTVHeap;
  wil::com_ptr<ID3D12DescriptorHeap> mD3DSRVHeap;
  wil::com_ptr<IDXGISwapChain1> mSwapChain;

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
  uint8_t mFrameIndex {};// Used to index into mFrames; reset when buffer reset

  // Device-independent pixels so that we keep the correct values when
  // dragging between monitors
  std::optional<SkSize> mMinimumContentSizeInDIPs;
  // Includes the non-client-area
  std::optional<SkISize> mMinimumWindowSize;

  DWORD mWindowStyle {};
  DWORD mWindowExStyle {};

  std::chrono::steady_clock::time_point mBeginFrameTime;

  void CreateNativeWindow(HINSTANCE);
  void InitializeD3D();
  void ConfigureD3DDebugLayer();

  void CreateRenderTargets();
  void CleanupFrameContexts();
  SkISize CalculateMinimumWindowSize();

  void EndFrame();

  LRESULT
  WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

  static LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
};
