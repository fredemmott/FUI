// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include <source_location>
#include <string>

namespace FredEmmott::GUI::win32_detail {
void ThrowHResult(
  HRESULT,
  const std::source_location& caller = std::source_location::current());
void CheckHResult(
  HRESULT,
  const std::source_location& caller = std::source_location::current());
std::wstring Utf8ToWide(std::string_view);
}// namespace FredEmmott::GUI::win32_detail
