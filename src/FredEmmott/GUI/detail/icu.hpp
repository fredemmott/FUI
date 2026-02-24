// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "FredEmmott/GUI/config.hpp"

#if (!defined(FUI_ENABLE_ICU)) && __has_include(<Windows.h>)
#include <Windows.h>
#endif

#if defined(FUI_ENABLE_ICU)
#include <unicode/ubrk.h>
#include <unicode/unum.h>
#include <unicode/utext.h>
#elif defined(NTDDI_VERSION) && NTDDI_VERSION >= NTDDI_WIN10_RS2
// Use ICU from the Windows SDK
static_assert(FredEmmott::GUI::Config::MinimumWindowsTarget >= NTDDI_VERSION);
#include <icucommon.h>
#include <icui18n.h>
#else
#error "No usable ICU header"
#endif

namespace FredEmmott::GUI::detail {

template <class T>
  requires(sizeof(T) == sizeof(UChar))
[[nodiscard]]
const UChar* uchar_cast(const T* const p) {
  return reinterpret_cast<const UChar*>(p);
}

template <class T>
  requires(sizeof(T) == sizeof(UChar))
[[nodiscard]]
UChar* uchar_cast(T* const p) {
  return reinterpret_cast<UChar*>(p);
}

}// namespace FredEmmott::GUI::detail
