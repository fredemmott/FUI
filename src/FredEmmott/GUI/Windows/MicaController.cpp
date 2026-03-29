// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "MicaController.hpp"

#include <dwmapi.h>

#include <memory>

#include "DirectCompositionController.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"

namespace FredEmmott::GUI {
using win32_detail::CheckHResult;

MicaController::MicaController(
  HWND const hwnd,
  IDXGISwapChain* const swapChain,
  const Kind kind)
  : mHwnd(hwnd),
    mKind(kind) {
  mParent = std::make_unique<DirectCompositionController>(hwnd, swapChain);

  this->ForceSetKind(kind);
}

MicaController::~MicaController() {
  static constexpr auto NoBackdrop = DWMSBT_NONE;
  DwmSetWindowAttribute(
    mHwnd, DWMWA_SYSTEMBACKDROP_TYPE, &NoBackdrop, sizeof(NoBackdrop));
}

void MicaController::SetKind(const Kind kind) {
  if (kind == mKind) {
    return;
  }
  this->ForceSetKind(kind);
}

void MicaController::ForceSetKind(const Kind kind) {
  mKind = kind;

  const auto backdropType
    = (kind == Kind::Mica) ? DWMSBT_MAINWINDOW : DWMSBT_TABBEDWINDOW;
  CheckHResult(DwmSetWindowAttribute(
    mHwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType)));
}

bool MicaController::IsSupported() noexcept {
  return DirectCompositionController::IsSupported();
}

}// namespace FredEmmott::GUI