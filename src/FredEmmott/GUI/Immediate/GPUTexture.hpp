// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

// GPUTexture today uses the Win32-shared-HANDLE ImportTexture path.
// Header Win32-gated to keep FredEmmott/GUI.hpp (which pulls in this
// header) parseable on Linux.
#ifdef _WIN32

#include "Button.hpp"
#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

using GPUTextureResult = Result<
  nullptr,
  void,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin>;

GPUTextureResult GPUTexture(
  ImportedTexture::HandleKind,
  HANDLE texture,
  HANDLE fence,
  uint64_t fenceValue,
  const Rect& sourceRect,
  const std::optional<Rect>& destRect = std::nullopt,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate

#endif// _WIN32