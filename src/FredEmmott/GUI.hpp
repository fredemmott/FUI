// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Button.hpp>
#include <FredEmmott/GUI/Immediate/Card.hpp>
#include <FredEmmott/GUI/Immediate/ComboBox.hpp>
#include <FredEmmott/GUI/Immediate/ComboBoxButton.hpp>
#include <FredEmmott/GUI/Immediate/ComboBoxItem.hpp>
#include <FredEmmott/GUI/Immediate/ComboBoxPopup.hpp>
#include <FredEmmott/GUI/Immediate/Disabled.hpp>
#include <FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp>
#include <FredEmmott/GUI/Immediate/FontIcon.hpp>
#include <FredEmmott/GUI/Immediate/Label.hpp>
#include <FredEmmott/GUI/Immediate/PopupWindow.hpp>
#include <FredEmmott/GUI/Immediate/ScrollView.hpp>
#include <FredEmmott/GUI/Immediate/StackPanel.hpp>
#include <FredEmmott/GUI/Immediate/TextBlock.hpp>
#include <FredEmmott/GUI/Immediate/ToggleSwitch.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <string_view>

#if __has_include(<FredEmmott/GUI/Windows/Win32Window.hpp>)
#include <FredEmmott/GUI/Windows/Win32Window.hpp>
#endif

namespace FredEmmott::GUI {

/** Human-readable description for debugging.
 *
 * If your code requires a specific backend, use:
 * - `skia_renderer_cast()`
 * - `direct2d_renderer_cast()`
 */
inline std::string_view GetBackendDescription() {
  return renderer_detail::GetRenderAPIDetails();
}

}// namespace FredEmmott::GUI