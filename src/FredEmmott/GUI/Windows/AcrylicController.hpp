// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <dxgi.h>

#include <FredEmmott/GUI/AcrylicBrush.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/Size.hpp>
#include <FredEmmott/GUI/WindowBackdrop.hpp>
#include <cinttypes>
#include <functional>
#include <memory>

namespace FredEmmott::GUI {
class Brush;

/// Re-implementation of WinUI3/Fluent "Acrylic" texture using
/// Windows::UI::Composition
class AcrylicController final {
 public:
  using Kind = Win32::Acrylic::Kind;
  AcrylicController() = delete;
  AcrylicController(const AcrylicController&) = delete;
  AcrylicController(AcrylicController&&) = delete;
  AcrylicController& operator=(const AcrylicController&) = delete;
  AcrylicController& operator=(AcrylicController&&) = delete;

  using CopySoftwareBitmap = std::function<void(
    IDXGISurface* dest,
    const BasicPoint<uint32_t>& destOffset,
    const void* inputData,
    const BasicSize<uint32_t>& inputSize,
    uint32_t inputStride)>;

  AcrylicController(
    Kind,
    const CopySoftwareBitmap&,
    IUnknown* device,
    HWND,
    IDXGISwapChain*,
    const BasicSize<uint32_t>&,
    const Brush&);
  ~AcrylicController();

  [[nodiscard]] static bool IsSupported() noexcept;

  void Resize(uint32_t width, uint32_t height);
  void SetBrush(const Brush&);

 private:
  // I generally dislike pimpl, but in this case, the Windows::UI::Composition
  // headers kill IDE performance (and in some case, correctness).
  //
  // Using pimpl here so that the headers stay constrained to
  // AcrylicController.cpp
  class Impl;
  std::unique_ptr<Impl> p;
};

}// namespace FredEmmott::GUI