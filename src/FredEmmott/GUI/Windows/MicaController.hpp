// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>
#include <dxgi.h>

#include <FredEmmott/GUI/WindowBackdrop.hpp>
#include <memory>

namespace FredEmmott::GUI {
class DirectCompositionController;

class MicaController final {
 public:
  using Kind = Win32::Mica::Kind;
 
  MicaController() = delete;
  MicaController(const MicaController&) = delete;
  MicaController(MicaController&&) = delete;
  MicaController& operator=(const MicaController&) = delete;
  MicaController& operator=(MicaController&&) = delete;

  MicaController(HWND, IDXGISwapChain*, Kind);
  ~MicaController();

  void SetKind(Kind);

  [[nodiscard]]
  static bool IsSupported() noexcept;

 private:
  void ForceSetKind(Kind);
  std::unique_ptr<DirectCompositionController> mParent;
  HWND mHwnd {};
  Kind mKind {};
};

}// namespace FredEmmott::GUI