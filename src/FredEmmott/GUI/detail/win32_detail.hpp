// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include <source_location>

namespace FredEmmott::GUI::win32_detail {
void ThrowHResult(
  HRESULT,
  const std::source_location& caller = std::source_location::current());
void CheckHResult(
  HRESULT,
  const std::source_location& caller = std::source_location::current());
}// namespace FredEmmott::GUI::win32_detail
