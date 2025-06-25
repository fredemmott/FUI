// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Window.hpp"

#include <thread>

#include "assert.hpp"
#include "detail/immediate_detail.hpp"

namespace FredEmmott::GUI {

Window::Window(
  const renderer_detail::RenderAPI renderAPI,
  const uint8_t swapChainLength)
  : mSwapChainLength(swapChainLength) {
  renderer_detail::SetRenderAPI(renderAPI);
}

std::expected<void, int> Window::BeginFrame() {
  // We may have failed since the last window message without it being directly
  // caused by a window message to this window.
  //
  // For example, popup windows can be closed by a click on their owner window.
  if (mExitCode.has_value()) {
    return std::unexpected {mExitCode.value()};
  }
  using namespace Immediate::immediate_detail;

  mBeginFrameTime = std::chrono::steady_clock::now();
  MSG msg {};
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (mExitCode.has_value()) {
      return std::unexpected {mExitCode.value()};
    }
  }
  FUI_ASSERT(!tWindow);
  tWindow = this;
  mFUIRoot.BeginFrame();
  return {};
}

void Window::WaitFrame(unsigned int minFPS, unsigned int maxFPS) const {
  if (minFPS == std::numeric_limits<unsigned int>::max()) {
    return;
  }

  if (mExitCode) {
    return;
  }

  const auto fps = std::clamp<unsigned int>(
    mFUIRoot.GetFrameRateRequirement() == FrameRateRequirement::SmoothAnimation
      ? 60
      : 0,
    minFPS,
    maxFPS);
  if (fps == 0) {
    MsgWaitForMultipleObjects(0, nullptr, false, INFINITE, QS_ALLINPUT);
  }
  std::chrono::milliseconds frameInterval {1000 / maxFPS};

  const auto frameDuration = std::chrono::steady_clock::now() - mBeginFrameTime;
  if (frameDuration >= frameInterval) {
    return;
  }
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
    frameInterval - frameDuration);

  std::this_thread::sleep_for(millis);
}

void Window::EndFrame() {
  using namespace Immediate::immediate_detail;
  mFUIRoot.EndFrame();

  FUI_ASSERT(tWindow == this, "Improperly nested windows");
  tWindow = nullptr;

  if (!this->GetNativeHandle()) [[unlikely]] {
    this->InitializeWindow();
    if (!this->GetNativeHandle()) {
      mExitCode = EXIT_FAILURE;
      return;
    }
  }

  this->Paint();
}

FrameRateRequirement Window::GetFrameRateRequirement() const {
  return mFUIRoot.GetFrameRateRequirement();
}

void Window::Paint() {
  this->ResizeIfNeeded();

  {
    const auto painter = this->GetFramePainter(mFrameIndex);
    const auto renderer = painter->GetRenderer();
    const auto layer = renderer->ScopedLayer();
    renderer->Clear(this->GetClearColor());
    renderer->Scale(this->GetDPIScale());
    mFUIRoot.Paint(renderer, this->GetClientAreaSize());
  }

  mFrameIndex = (mFrameIndex + 1) % mSwapChainLength;
}

}// namespace FredEmmott::GUI
