// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "win32_detail.hpp"

#include <bit>
#include <format>
#include <functional>
#include <system_error>

#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/config.hpp"

namespace FredEmmott::GUI::win32_detail {

namespace {
constexpr std::size_t StaticBufferSize = 1024 * 1024;

template <std::integral T>
T size_cast(const std::size_t v) {
  if (v > std::numeric_limits<T>::max()) {
    throw std::overflow_error("size too large");
  }
  return static_cast<T>(v);
}

template <class T, class TFromStringView, class TToString, class TSizeType>
concept text_transcoder = std::is_invocable_r_v<
  TSizeType,
  T,
  typename TFromStringView::value_type const*,
  TSizeType,
  typename TToString::value_type*,
  TSizeType>;

template <
  class TFromStringView,
  class TToString,
  text_transcoder<TFromStringView, TToString, int> auto TImpl>
TToString ConvertEncoding(const TFromStringView s) {
  if (s.empty()) {
    return {};
  }

  const auto convert
    = std::bind_front(TImpl, s.data(), size_cast<int>(s.size()));

  // If we can fit in `StaticBufferSize`, avoid a heap allocation
  thread_local TToString tlBuffer;
  // Reserves size + 1 for trailing null
  // Let's make StaticBufferSize the *actual* buffer size. Especially as
  // powers-of-two are normal, going one byte over is likely to increase
  // fragmentation and may hurt performance if it turns out to be page size + 1
  tlBuffer.resize_and_overwrite(StaticBufferSize - 1, convert);
  if (!tlBuffer.empty()) {
    FUI_ASSERT(tlBuffer.back() != '\0' || s.back() == '\0');
    return tlBuffer;
  }

  const auto charCount = convert(nullptr, 0);
  const auto bufferSize = charCount + 1;
  TToString ret;
  ret.resize_and_overwrite(bufferSize, convert);
  FUI_ASSERT(ret.back() != '\0' || s.back() == '\0');
  return ret;
}

}// namespace

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
  constexpr auto Impl = []<class... TArgs>(TArgs&&... args) {
    return MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, std::forward<TArgs>(args)...);
  };
  return ConvertEncoding<std::string_view, std::wstring, Impl>(s);
}

std::string WideToUtf8(const std::wstring_view s) {
  constexpr auto Impl = []<class... TArgs>(TArgs&&... args) {
    return WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      std::forward<TArgs>(args)...,
      nullptr,
      nullptr);
  };

  return ConvertEncoding<std::wstring_view, std::string, Impl>(s);
}

}// namespace FredEmmott::GUI::win32_detail