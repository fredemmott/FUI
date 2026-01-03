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
std::string WideToUtf8(std::wstring_view);

inline std::size_t WideToUtf8Index(
  const std::wstring_view w,
  const size_t wIndex) {
  if (wIndex == 0) {
    return 0;
  }
  return WideCharToMultiByte(
    CP_UTF8,
    WC_ERR_INVALID_CHARS,
    w.data(),
    wIndex,
    nullptr,
    0,
    nullptr,
    nullptr);
}

inline std::size_t Utf8ToWideIndex(
  const std::string_view u8,
  const size_t u8Index) {
  if (u8Index == 0) {
    return 0;
  }
  return MultiByteToWideChar(
    CP_UTF8, MB_ERR_INVALID_CHARS, u8.data(), u8Index, nullptr, 0);
}
}// namespace FredEmmott::GUI::win32_detail
