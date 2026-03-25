// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::Immediate {

/** True if an item was clicked or activated via some other method.
 *
 * For example, a button can also be activated by the space bar.
 */
[[nodiscard]]
bool WasActivated() noexcept;
/// True if an item was right-clicked, or similarly activated via some other
/// method.
[[nodiscard]]
bool WasContextActivated() noexcept;

}// namespace FredEmmott::GUI::Immediate