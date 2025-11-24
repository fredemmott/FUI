// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemSettings.hpp"

namespace FredEmmott::GUI {
namespace {

template <class TIn, class TOut, UINT Win32Key>
struct ConvertSetting {
  static constexpr TOut operator()(TIn&& value) {
    return static_cast<TOut>(value);
  }
};

template <UINT Win32Key>
struct ConvertSetting<HIGHCONTRASTW, bool, Win32Key> {
  static constexpr BOOL operator()(HIGHCONTRASTW&& value) {
    return (value.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON;
  }
};

template <class TWin32, UINT Win32Key, class TRet>
TRet GetSetting(std::optional<TRet>& storage) {
  if (storage) {
    return *storage;
  }
  TWin32 value {};
  SystemParametersInfoW(Win32Key, 0, &value, 0);
  storage = ConvertSetting<TWin32, TRet, Win32Key>()(std::move(value));
  return *storage;
}

template <>
struct ConvertSetting<
  INT,
  std::chrono::steady_clock::duration,
  SPI_GETKEYBOARDDELAY> {
  static constexpr std::chrono::steady_clock::duration operator()(
    const INT value) {
    // '3' is approximately 1 second (1 Hz)
    // '0' is approximately 250 ms (4 Hz)

    constexpr auto Min = std::chrono::milliseconds(250);
    constexpr auto Ratio = (std::chrono::seconds(1) - Min) / 3;
    return Min + (value * Ratio);
  }
};

template <>
struct ConvertSetting<
  DWORD,
  std::chrono::steady_clock::duration,
  SPI_GETKEYBOARDSPEED> {
  static constexpr std::chrono::steady_clock::duration operator()(
    const INT value) {
    // 31 is approximately 30 Hz
    // 0 is approximately 2.5 Hz

    constexpr auto MinHz = 2.5;
    constexpr auto MaxHz = 30;
    constexpr auto MinTime = std::chrono::milliseconds(1000) / MaxHz;
    constexpr auto MaxTime = std::chrono::milliseconds(1000) / MinHz;
    constexpr auto Ratio = (MaxTime - MinTime) / 31;
    return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      MaxTime - (value * Ratio));
  }
};

}// namespace

SystemSettings::SystemSettings() = default;

SystemSettings& SystemSettings::Get() {
  static SystemSettings sInstance;
  return sInstance;
}

void SystemSettings::ClearWin32(const UINT key) {
  switch (key) {
#define F(RETURN_TYPE, NAME, WIN32_TYPE, WIN32_GET, WIN32_SET) \
  case WIN32_GET: \
  case WIN32_SET: /* for WM_SETTINGCHANGE message */ \
    m##NAME.reset(); \
    return;
    FUI_ENUM_SYSTEM_SETTINGS(F);
#undef F
    default:
      // not an SPI setting - but need a statement here
      break;
  }
}

#define DEFINE_GETTER(RETURN_TYPE, NAME, WIN32_TYPE, WIN32_GET, WIN32_SET) \
  RETURN_TYPE SystemSettings::Get##NAME() const { \
    return GetSetting<WIN32_TYPE, WIN32_GET>(m##NAME); \
  }
FUI_ENUM_SYSTEM_SETTINGS(DEFINE_GETTER)
#undef DEFINE_GETTER

std::optional<std::chrono::steady_clock::duration>
SystemSettings::GetCaretBlinkInterval() const {
  const auto millis = ::GetCaretBlinkTime();
  if (millis == INFINITE) {
    return std::nullopt;
  }
  return std::chrono::milliseconds(millis);
};

}// namespace FredEmmott::GUI