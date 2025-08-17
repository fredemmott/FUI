// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Window.hpp"

#include <FredEmmott/GUI/StaticTheme/Generic.hpp>
#include <thread>

#include "FredEmmott/GUI/events/KeyEvent.hpp"
#include "Immediate/ContentDialog.hpp"
#include "assert.hpp"
#include "detail/immediate_detail.hpp"

namespace FredEmmott::GUI {

Window::Window(const uint8_t swapChainLength)
  : mSwapChainLength(swapChainLength) {}

std::expected<void, int> Window::BeginFrame() {
  std::call_once(mGraphicsAPIFlag, [this]() { this->InitializeGraphicsAPI(); });
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
    this->WaitForInput();
  }
  std::chrono::milliseconds frameInterval {1000 / maxFPS};

  const auto frameDuration = std::chrono::steady_clock::now() - mBeginFrameTime;
  if (frameDuration >= frameInterval) {
    return;
  }
  this->InterruptableWait(frameInterval - frameDuration);
}

void Window::EndFrame() {
  using namespace Immediate::immediate_detail;
  mFUIRoot.EndFrame();

  FUI_ASSERT(tWindow == this, "Improperly nested windows");
  const auto resetWindowAtExit = wil::scope_exit([] { tWindow = nullptr; });

  if (mExitCode) {
    this->HideWindow();
    return;
  }

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

void Window::SetDefaultAction(const std::function<void()>& action) {
  mDefaultAction = action;
}

void Window::SetCancelAction(const std::function<void()>& action) {
  mCancelAction = action;
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
    if (IsDisabled()) {
      renderer->FillRect(
        *StaticTheme::Common::SmokeFillColorDefaultBrush->Resolve(),
        this->GetClientAreaSize());
    }
  }

  mFrameIndex = (mFrameIndex + 1) % mSwapChainLength;
}

void Window::ResetToFirstBackBuffer() {
  mFrameIndex = 0;
}

Widgets::Widget* Window::DispatchEvent(const MouseEvent& e) {
  const auto fm = GetRoot()->GetFocusManager();
  FocusManager::PushInstance(fm);
  const auto popFocusManager
    = wil::scope_exit([&]() { FocusManager::PopInstance(fm); });

  return GetRoot()->GetWidget()->DispatchEvent(e);
}

void Window::DispatchEvent(const KeyEvent& e) {
  const auto fm = GetRoot()->GetFocusManager();
  FocusManager::PushInstance(fm);
  const auto popFocusManager
    = wil::scope_exit([&]() { FocusManager::PopInstance(fm); });

  if (GetRoot()->GetWidget()->DispatchEvent(e)) {
    return;
  }
  const auto pe = dynamic_cast<KeyPressEvent const*>(&e);
  if (!pe) {
    return;
  }
  if (fm->OnKeyPress(*pe)) {
    return;
  }

  using enum KeyCode;
  switch (pe->mKeyCode) {
    case Key_Escape:
      if (mCancelAction) {
        mCancelAction();
        return;
      }
      if (this->IsPopup()) {
        this->RequestStop(EXIT_SUCCESS);
      }
      return;
    case Key_Return:
      if (mDefaultAction) {
        mDefaultAction();
      }
      return;
    default:
      break;
  }
}

void Window::DispatchEvent(const TextInputEvent& e) {
  const auto fm = GetRoot()->GetFocusManager();
  FocusManager::PushInstance(fm);
  const auto popFocusManager
    = wil::scope_exit([&]() { FocusManager::PopInstance(fm); });

  (void)GetRoot()->GetWidget()->DispatchEvent(e);
}

}// namespace FredEmmott::GUI
