// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "win32_detail.hpp"

#include <bit>
#include <format>
#include <system_error>

namespace FredEmmott::GUI::win32_detail {
void ThrowHResult(const HRESULT ret, const std::source_location& caller) {
  const std::error_code ec {ret, std::system_category()};

  const auto msg = std::format(
    "HRESULT failed: {:#010x} @ {} - {}:{}:{} - {}\n",
    std::bit_cast<uint32_t>(ret),
    caller.function_name(),
    caller.file_name(),
    caller.line(),
    caller.column(),
    ec.message());
  OutputDebugStringA(msg.c_str());
  throw std::system_error(ec, msg);
}

void CheckHResult(const HRESULT ret, const std::source_location& caller) {
  if (SUCCEEDED(ret)) [[likely]] {
    return;
  }
  ThrowHResult(ret, caller);
}

std::wstring Utf8ToWide(const std::string_view s) {
  const auto retCharCount = MultiByteToWideChar(
    CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), s.size(), nullptr, 0);
  std::wstring ret;
  ret.resize(retCharCount);
  MultiByteToWideChar(
    CP_UTF8,
    MB_ERR_INVALID_CHARS,
    s.data(),
    s.size(),
    ret.data(),
    retCharCount);
  if (const auto i = ret.find_last_of(L'\0'); i != std::wstring::npos) {
    ret.erase(i);
  }
  return ret;
}

std::string WideToUtf8(const std::wstring_view s) {
  const auto retCharCount = WideCharToMultiByte(
    CP_UTF8,
    WC_ERR_INVALID_CHARS,
    s.data(),
    s.size(),
    nullptr,
    0,
    nullptr,
    nullptr);
  std::string ret;
  ret.resize(retCharCount);
  WideCharToMultiByte(
    CP_UTF8,
    WC_ERR_INVALID_CHARS,
    s.data(),
    s.size(),
    ret.data(),
    retCharCount,
    nullptr,
    nullptr);
  if (const auto i = ret.find_last_of('\0'); i != std::string::npos) {
    ret.erase(i);
  }
  return ret;
}

}// namespace FredEmmott::GUI::win32_detail