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
  Window(renderer_detail::RenderAPI, uint8_t swapChainLength);
  virtual ~Window() = default;

  virtual std::unique_ptr<Window> CreatePopup() const = 0;
  virtual void SetParent(NativeHandle) = 0;
  virtual NativeHandle GetNativeHandle() const noexcept = 0;
  virtual void SetInitialPositionInNativeCoords(const NativePoint& native) = 0;
  virtual void OffsetPositionToDescendant(Widgets::Widget* child) = 0;
  virtual NativePoint CanvasPointToNativePoint(const Point& canvas) const = 0;

  [[nodiscard]]
  std::expected<void, int> BeginFrame();
  void WaitFrame(unsigned int minFPS = 0, unsigned int maxFPS = 60) const;
  void EndFrame();
  FrameRateRequirement GetFrameRateRequirement() const;

 protected:
  class BasicFramePainter {
   public:
    virtual ~BasicFramePainter() = default;

    virtual Renderer* GetRenderer() noexcept = 0;
  };

  virtual void InitializeWindow() = 0;
  virtual std::unique_ptr<BasicFramePainter> GetFramePainter(
    uint8_t mFrameIndex)
    = 0;
  virtual void ResizeIfNeeded() = 0;
  virtual Size GetClientAreaSize() const = 0;
  virtual float GetDPIScale() const = 0;
  virtual Color GetClearColor() const = 0;

  // This is protected so it can be called outside the usual frame loop, e.g.
  // when resizing on Windows
  void Paint();

  auto GetRoot() const noexcept {
    return &mFUIRoot;
  }

  void DispatchEvent(Event* e) {
    mFUIRoot.DispatchEvent(e);
  }

  void RequestStop(int exitCode = EXIT_SUCCESS) {
    mExitCode = exitCode;
  }

 private:
  uint8_t mSwapChainLength {};
  std::chrono::steady_clock::time_point mBeginFrameTime;
  uint8_t mFrameIndex {};// Used to index into mFrames; reset when buffer reset

  std::optional<int> mExitCode;
  Immediate::Root mFUIRoot;
};

}// namespace FredEmmott::GUI