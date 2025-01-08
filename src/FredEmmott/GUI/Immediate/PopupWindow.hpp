// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

/** Start a popup window.
 *
 * Returns true if it's visible and content should follow.
 *
 * Example usage:
 *
 *   static bool showPopup = false;
 *   if (fui::Button("Show")) {
 *     showPopup = true;
 *   }
 *   if (showPopup && fui::BeginPopupWindow()) {
 *     // popup content goes here
 *   } else {
 *     // Window closed
 *     showPopup = false;
 *   }
 *
 */
[[nodiscard]]
bool BeginPopupWindow(ID id = ID {std::source_location::current()});
/** Start a popup window; optionally show it.
 *
 * Returns true if the window is open and content should follow.
 *
 * @param open inout: if true, the window will be shown. When the window is
 *   closed, it will be set to false.
 */
[[nodiscard]]
bool BeginPopupWindow(bool* open, ID id = ID {std::source_location::current()});
void EndPopupWindow();

}// namespace FredEmmott::GUI::Immediate