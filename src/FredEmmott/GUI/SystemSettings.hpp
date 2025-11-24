// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include <chrono>
#include <cinttypes>
#include <optional>

namespace FredEmmott::GUI {

// TODO (C++26): replace with members and reflection
// X(RETURN_TYPE, NAME, WIN32_TYPE, WIN32_GET, WIN32_SET)
#define FUI_ENUM_SYSTEM_SETTINGS(X) \
  X(bool, \
    AnimationsEnabled, \
    BOOL, \
    SPI_GETCLIENTAREAANIMATION, \
    SPI_SETCLIENTAREAANIMATION) \
  X(uint32_t, CaretWidth, DWORD, SPI_GETCARETWIDTH, SPI_SETCARETWIDTH) \
  X(bool, \
    HighContrast, \
    HIGHCONTRASTW, \
    SPI_GETHIGHCONTRAST, \
    SPI_SETHIGHCONTRAST) \
  X(std::chrono::steady_clock::duration, \
    KeyboardRepeatDelay, \
    INT, \
    SPI_GETKEYBOARDDELAY, \
    SPI_SETKEYBOARDDELAY) \
  X(std::chrono::steady_clock::duration, \
    KeyboardRepeatInterval, \
    DWORD, \
    SPI_GETKEYBOARDSPEED, \
    SPI_SETKEYBOARDSPEED) \
  X(uint32_t, \
    MouseWheelScrollChars, \
    UINT, \
    SPI_GETWHEELSCROLLCHARS, \
    SPI_SETWHEELSCROLLCHARS) \
  X(uint32_t, \
    MouseWheelScrollLines, \
    UINT, \
    SPI_GETWHEELSCROLLLINES, \
    SPI_SETWHEELSCROLLLINES)

class SystemSettings {
 public:
  SystemSettings(const SystemSettings&) = delete;
  SystemSettings& operator=(const SystemSettings&) = delete;
  static SystemSettings& Get();

#define FUI_DECLARE_WINDOWS_SYSTEM_SETTING_GETTER( \
  RETURN_TYPE, NAME, WIN32_TYPE, WIN32_GET, WIN32_SET) \
  RETURN_TYPE Get##NAME() const;
  FUI_ENUM_SYSTEM_SETTINGS(FUI_DECLARE_WINDOWS_SYSTEM_SETTING_GETTER);
#undef FUI_DECLARE_WINDOWS_SYSTEM_SETTING_GETTER

  std::optional<std::chrono::steady_clock::duration> GetCaretBlinkInterval()
    const;

  void ClearWin32(UINT key);

 protected:
  SystemSettings();

 private:
#define FUI_DECLARE_WINDOWS_SYSTEM_SETTING_STORAGE( \
  RETURN_TYPE, NAME, WIN32_TYPE, WIN32_GET, WIN32_SET) \
  mutable std::optional<RETURN_TYPE> m##NAME;
  FUI_ENUM_SYSTEM_SETTINGS(FUI_DECLARE_WINDOWS_SYSTEM_SETTING_STORAGE);
#undef FUI_DECLARE_WINDOWS_SYSTEM_SETTING_STORAGE
};

}// namespace FredEmmott::GUI