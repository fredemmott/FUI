// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <dcomp.h>
#include <wil/com.h>

#include "FredEmmott/GUI/AcrylicBrush.hpp"

namespace FredEmmott::GUI {

class DirectCompositionController final {
 public:
  DirectCompositionController() = delete;
  DirectCompositionController(const DirectCompositionController&) = delete;
  DirectCompositionController(DirectCompositionController&&) = delete;
  DirectCompositionController& operator=(const DirectCompositionController&)
    = delete;
  DirectCompositionController& operator=(DirectCompositionController&&)
    = delete;

  DirectCompositionController(HWND, IDXGISwapChain*);
  ~DirectCompositionController();

  [[nodiscard]]
  static bool IsSupported() noexcept;

 private:
  wil::com_ptr<IDCompositionDevice> mDevice;
  wil::com_ptr<IDCompositionTarget> mTarget;
  wil::com_ptr<IDCompositionVisual> mVisual;
};

}// namespace FredEmmott::GUI