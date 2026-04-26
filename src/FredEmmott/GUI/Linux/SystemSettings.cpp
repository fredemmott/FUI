// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Phase 1.4 Linux stub. Hard-coded defaults chosen to match Windows factory
// settings so widgets behave predictably. A real impl (phase 4) will read
// portal Settings / gsettings.

#include "../SystemSettings.hpp"

namespace FredEmmott::GUI {

using namespace std::chrono_literals;

SystemSettings::SystemSettings() = default;

SystemSettings& SystemSettings::Get() {
  static SystemSettings sInstance;
  return sInstance;
}

bool SystemSettings::GetAnimationsEnabled() const {
  return true;
}

uint32_t SystemSettings::GetCaretWidth() const {
  return 1;
}

bool SystemSettings::GetHighContrast() const {
  return false;
}

std::chrono::steady_clock::duration SystemSettings::GetKeyboardRepeatDelay()
  const {
  return 500ms;
}

std::chrono::steady_clock::duration SystemSettings::GetKeyboardRepeatInterval()
  const {
  return 33ms;
}

uint32_t SystemSettings::GetMouseWheelScrollChars() const {
  return 3;
}

uint32_t SystemSettings::GetMouseWheelScrollLines() const {
  return 3;
}

std::optional<std::chrono::steady_clock::duration>
SystemSettings::GetCaretBlinkInterval() const {
  return 530ms;
}

bool SystemSettings::IsTransparencyEnabled() const {
  return false;
}

}// namespace FredEmmott::GUI
