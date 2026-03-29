// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "DirectCompositionController.hpp"

#include <mutex>

#include "FredEmmott/GUI/detail/win32_detail.hpp"

namespace FredEmmott::GUI {

using win32_detail::CheckHResult;

DirectCompositionController::DirectCompositionController(
  HWND const hwnd,
  IDXGISwapChain* swapChain) {
  CheckHResult(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(mDevice.put())));

  CheckHResult(mDevice->CreateVisual(mVisual.put()));
  CheckHResult(mVisual->SetContent(swapChain));

  CheckHResult(mDevice->CreateTargetForHwnd(hwnd, TRUE, mTarget.put()));
  CheckHResult(mTarget->SetRoot(mVisual.get()));

  CheckHResult(mDevice->Commit());
}

DirectCompositionController::~DirectCompositionController() = default;

bool DirectCompositionController::IsSupported() noexcept {
  static bool ret {};
  static std::once_flag once {};
  std::call_once(once, [&p = ret] {
    wil::com_ptr<IDCompositionDevice> device;
    p = SUCCEEDED(
      DCompositionCreateDevice(nullptr, IID_PPV_ARGS(device.put())));
  });
  return ret;
}

}// namespace FredEmmott::GUI
