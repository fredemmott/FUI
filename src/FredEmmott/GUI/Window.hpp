// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/detail/renderer_detail.hpp>
#include <chrono>
#include <expected>
#include <memory>

#include "Immediate/Root.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}// namespace FredEmmott::GUI::Widgets

namespace FredEmmott::GUI {

class Renderer;

class Window {
 public:
  struct NativeHandle {
    HWND mValue {};
    constexpr operator HWND() const noexcept {
      return mValue;
    }

    constexpr operator bool() const noexcept {
      return mValue != nullptr;
    }
  };
  enum class ResizeMode {
    Fixed = 0,
    AllowShrink = 1,
    AllowGrow = 2,
    Allow = AllowShrink | AllowGrow,
  };
  Window(uint8_t swapChainLength);
  virtual ~Window() = default;

  [[nodiscard]]
  virtual std::unique_ptr<Window> CreatePopup() const
    = 0;
  virtual void SetParent(NativeHandle) = 0;
  [[nodiscard]]
  virtual NativeHandle GetNativeHandle() const noexcept
    = 0;
  virtual void SetInitialPositionInNativeCoords(const NativePoint& native) = 0;
  virtual void OffsetPositionToDescendant(Widgets::Widget* child) = 0;
  /// Changes the size as little as possible to meet the constraints
  virtual void ApplySizeConstraints() = 0;
  /// Resize to the 'ideal' size
  virtual void ResizeToIdeal() = 0;
  [[nodiscard]]
  virtual bool IsDisabled() const
    = 0;
  [[nodiscard]]
  virtual NativePoint CanvasPointToNativePoint(const Point& canvas) const
    = 0;
  virtual void SetResizeMode(ResizeMode horizontal, ResizeMode vertical) = 0;

  /// Start a frame, or provide an exit code.
  [[nodiscard]]
  std::expected<void, int> BeginFrame();
  void WaitFrame(unsigned int minFPS = 0, unsigned int maxFPS = 60) const;
  void EndFrame();
  [[nodiscard]]
  FrameRateRequirement GetFrameRateRequirement() const;
  virtual void InterruptWaitFrame() = 0;

  void RequestStop(int exitCode = EXIT_SUCCESS) {
    mExitCode = exitCode;
    this->InterruptWaitFrame();
  }

 protected:
  class BasicFramePainter {
   public:
    virtual ~BasicFramePainter() = default;

    virtual Renderer* GetRenderer() noexcept = 0;
  };

  virtual void InitializeWindow() = 0;
  virtual void HideWindow() = 0;
  virtual std::unique_ptr<BasicFramePainter> GetFramePainter(
    uint8_t mFrameIndex)
    = 0;
  virtual void ResizeIfNeeded() = 0;
  virtual Size GetClientAreaSize() const = 0;
  virtual float GetDPIScale() const = 0;
  virtual Color GetClearColor() const = 0;
  virtual void InitializeGraphicsAPI() = 0;
  virtual void WaitForInput() const = 0;
  /// Wait for the specified amount of time, unless a new frame is requested
  /// sooner
  virtual void InterruptableWait(
    const std::chrono::steady_clock::duration&) const
    = 0;

  // This is protected so it can be called outside the usual frame loop, e.g.
  // when resizing on Windows
  void Paint();
  std::optional<int> GetExitCode() const noexcept {
    return mExitCode;
  }

  auto GetRoot() const noexcept {
    return &mFUIRoot;
  }

  Widgets::Widget* DispatchEvent(Event* e) {
    return mFUIRoot.DispatchEvent(e);
  }

  void ResetToFirstBackBuffer();

 private:
  uint8_t mSwapChainLength {};
  std::chrono::steady_clock::time_point mBeginFrameTime;
  uint8_t mFrameIndex {};// Used to index into mFrames; reset when buffer reset
  std::once_flag mGraphicsAPIFlag;

  std::optional<int> mExitCode;
  Immediate::Root mFUIRoot;
};

}// namespace FredEmmott::GUI