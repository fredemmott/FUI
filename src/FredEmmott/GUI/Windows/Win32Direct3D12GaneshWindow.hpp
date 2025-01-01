// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <Windows.h>
#include <core/SkCanvas.h>
#include <d3d12.h>
#include <dcomp.h>
#include <dxgi1_4.h>
#include <skia/gpu/GrDirectContext.h>
#include <wil/com.h>
#include <wil/resource.h>

#include <FredEmmott/GUI/Immediate/Root.hpp>
#include <expected>
#include <optional>

struct WindowOptions {
  std::string mTitle;

  /** Initial window size
   *
   * 0 = computed minimum width for content
   * You may also want CW_USEDEFAULT.
   */
  SkISize mInitialSize {};

  std::string mClass;
  HWND mParentWindow {nullptr};
};

class Win32Direct3D12GaneshWindow final {
 public:
  using Options = WindowOptions;
  Win32Direct3D12GaneshWindow() = delete;
  Win32Direct3D12GaneshWindow(const Win32Direct3D12GaneshWindow&) = delete;
  Win32Direct3D12GaneshWindow(Win32Direct3D12GaneshWindow&&) = delete;
  Win32Direct3D12GaneshWindow& operator=(const Win32Direct3D12GaneshWindow&)
    = delete;
  Win32Direct3D12GaneshWindow& operator=(Win32Direct3D12GaneshWindow&&)
    = delete;

  explicit Win32Direct3D12GaneshWindow(
    HINSTANCE instance,
    int showCommand,
    const Options& options = {});
  ~Win32Direct3D12GaneshWindow();

  [[nodiscard]] HWND GetHWND() const noexcept;

  [[nodiscard]]
  std::expected<void, int> BeginFrame();
  void WaitFrame(
    unsigned int minFPS = 0,
    unsigned int maxFPS = std::numeric_limits<unsigned int>::max()) const;
  void EndFrame();

 private:
  static constexpr UINT SwapChainLength = 3;
  static thread_local std::unordered_map<HWND, Win32Direct3D12GaneshWindow*>
    gInstances;
  static thread_local Win32Direct3D12GaneshWindow* gInstanceCreatingWindow;

  HINSTANCE mInstanceHandle {nullptr};
  int mShowCommand {SW_SHOW};
  Options mOptions {};

  std::optional<int> mExitCode;

  wil::unique_hwnd mHwnd;
  float mDPIScale = {1.0f};
  std::optional<DWORD> mDPI;
  SkISize mWindowSize {};
  std::optional<SkISize> mPendingResize;

  FredEmmott::GUI::Immediate::Root mFUIRoot;

  wil::com_ptr<IDCompositionDevice> mCompositionDevice;
  wil::com_ptr<IDCompositionTarget> mCompositionTarget;
  wil::com_ptr<IDCompositionVisual> mCompositionVisual;
  bool mHaveSystemBackdrop {false};

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

  void AdjustToWindowsTheme();
  void InitializeWindow();
  void CreateNativeWindow();
  void InitializeD3D();
  void InitializeSkia();
  void ConfigureD3DDebugLayer();

  void CreateRenderTargets();
  void CleanupFrameContexts();
  SkISize CalculateMinimumWindowSize();
  void ResizeIfNeeded();

  LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  static LRESULT
  StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
