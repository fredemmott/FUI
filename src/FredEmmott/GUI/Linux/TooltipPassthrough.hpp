// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

struct SDL_Window;

namespace FredEmmott::GUI::Linux {

// Sets an empty input region on the SDL window's underlying surface so it
// doesn't intercept the cursor (Win32's WS_EX_TRANSPARENT analogue).
// Call once after SDL_CreatePopupWindow when the popup is a tooltip.
void MakePopupInputPassthrough(SDL_Window* window);

// Wayland's input region is double-buffered surface state and SDL3 may
// reset it during its own surface setup. Calling this every frame for a
// tooltip popup re-stages the empty region so the next SDL3 commit will
// deliver it. No-op on X11 (ShapeInput is sticky once set).
void RestakeTooltipInputRegion(SDL_Window* window);

}// namespace FredEmmott::GUI::Linux
