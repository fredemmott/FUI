// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {
struct BasicPopupWindowResultMixin {
  template <class Self>
  decltype(auto) Transparent(this Self&& self, bool transparent = true) {
    if (self.GetValue()) {
      Self::MakeTransparent(transparent);
    }
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Modal(this Self&& self, bool modal = true) {
    if (self.GetValue()) {
      Self::MakeModal(modal);
    }
    return std::forward<Self>(self);
  }

 private:
  static void MakeTransparent(bool transparent);
  static void MakeModal(bool modal);
};
}// namespace immediate_detail

void EndBasicPopupWindow();

using BasicPopupWindowResult = Result<
  &EndBasicPopupWindow,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin,
  immediate_detail::BasicPopupWindowResultMixin>;

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
BasicPopupWindowResult BeginBasicPopupWindow(
  ID id = ID {std::source_location::current()});
/** Start a popup window; optionally, show it.
 *
 * Returns true if the window is open and content should follow.
 *
 * @param open inout: if true, the window will be shown. When the window is
 *   closed, it will be set to false.
 */
[[nodiscard]]
BasicPopupWindowResult BeginBasicPopupWindow(
  bool* open,
  ID id = ID {std::source_location::current()});

void ClosePopupWindow();

void EndPopup();
using PopupResult = Result<
  &EndPopup,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin>;

[[nodiscard]]
PopupResult BeginPopup(ID id = ID {std::source_location::current()});
[[nodiscard]]
PopupResult BeginPopup(
  bool* open,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate