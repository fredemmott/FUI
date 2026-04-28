// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Mouse-passthrough for SDL3 popup windows used as tooltips. Win32 gets this
// via WS_EX_TRANSPARENT; on Linux there's no SDL3-level equivalent, so the
// Linux backend sets an empty input region on the underlying surface
// directly. Other SdlWindow platforms (future macOS / Windows-via-SDL) must
// supply their own implementation of these symbols (or a no-op TU) when
// added.
//
// Declared in this generic SDL-detail header so SdlWindow stays free of
// per-platform includes; the Linux implementation lives in
// FredEmmott/GUI/Linux/TooltipPassthrough.cpp because <X11/Xlib.h>'s global
// namespace pollution requires an isolated translation unit.
#pragma once

struct SDL_Window;

namespace FredEmmott::GUI::sdl_detail {

// Sets an empty input region on the SDL window's underlying surface so it
// doesn't intercept the cursor. Call once after SDL_CreatePopupWindow when
// the popup is a tooltip.
void MakePopupInputPassthrough(SDL_Window* window);

// Wayland's input region is double-buffered surface state and SDL3 may
// reset it during its own surface setup. Calling this every frame for a
// tooltip popup re-stages the empty region so the next SDL3 commit will
// deliver it. No-op on X11 (ShapeInput is sticky once set).
void RestakeTooltipInputRegion(SDL_Window* window);

}// namespace FredEmmott::GUI::sdl_detail
