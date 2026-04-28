# Linux backend

Linux-specific implementations of FUI's platform abstractions, mirroring
[`Windows/`](../Windows/) where a Linux body has to replace a Win32 one
(themes, settings, fonts, icons, IME-aware text input). Every file here
corresponds to a Win32-only body that gates on `_WIN32`; the Linux file
is selected instead when building on Linux (see [`src/lib.cmake`](../../../lib.cmake)).

The cross-platform-eligible **SDL3 windowing base + Skia/Vulkan window
subclass** live in [`Sdl/`](../Sdl/), not here — those classes are the
parents of any future macOS / SDL-on-Windows variants and aren't
inherently Linux. Only the genuinely-Linux pieces (Wayland/X11
specifics, fontconfig wiring, xdg-desktop-portal calls) belong in this
directory.

## Module map

| File | Provides | Status |
|---|---|---|
| [`Font.cpp`](Font.cpp) | `Font` ctor/`WithSize`/`WithWeight`/`MeasureTextWidth` — Skia-only path (no DirectWrite). Different from Windows in one place: an empty/fallback `SkFont` stays a real `SkFont` rather than collapsing to `monostate`, because fontconfig substitutes don't always exist for Segoe families. | Working. |
| [`SystemTheme.cpp`](SystemTheme.cpp) | `SystemTheme::Resolve` for sys-color and accent slots. Stub: hard-coded Win11 Light palette. | Stub. Real impl reads `xdg-desktop-portal` color-scheme + accent. |
| [`StaticTheme.cpp`](StaticTheme.cpp) | `StaticTheme::GetCurrent` and override stack. Stub: returns `Theme::Light`. | Stub. Real impl subscribes to portal Settings signals for live switching. |
| [`SystemSettings.cpp`](SystemSettings.cpp) | `SystemSettings::Get*` (caret blink, key repeat, scroll lines, animations). Stub: hard-coded values matching Windows factory defaults so widgets behave predictably. | Stub. Real impl reads portal Settings / gsettings. |
| [`IconProvider.cpp`](IconProvider.cpp) | `ApplicationIconProvider` impl. Stub: always reports invalid; `TitleBar` then renders no icon. | Stub. Real impl hooks freedesktop icon themes via Gio / xdg-icon-resource. |
| [`NumberBox.cpp`](NumberBox.cpp) | Immediate-mode `NumberBox(float*)` — minimum-viable two-way binding with `std::from_chars` parsing and `%g` formatting. Replaces the cross-platform [`Immediate/NumberBox.cpp`](../Immediate/NumberBox.cpp) which is `wchar_t`/`UChar`-coupled. Handles the focus-ping-pong and external-update edge cases inline. | MVP. Replace once cross-platform NumberBox is portable. |
| [`TextBox.cpp`](TextBox.cpp) | Widget-level `TextBox`: UTF-8 append on text-input events, backspace, click→focus, paint. Replaces the cross-platform [`Widgets/TextBox.cpp`](../Widgets/TextBox.cpp) which is Win32-TSF-coupled. | MVP. Selection / cursor positioning / IME preedit / undo-redo are not yet implemented. |
| [`TooltipPassthrough.cpp`](TooltipPassthrough.cpp) | Linux implementation of the `sdl_detail::{MakePopupInputPassthrough, RestakeTooltipInputRegion}` interface declared in [`detail/sdl_detail/PopupInputPassthrough.hpp`](../detail/sdl_detail/PopupInputPassthrough.hpp). Sets an empty input region on a tooltip popup's underlying surface so it doesn't intercept the cursor (Win32's `WS_EX_TRANSPARENT` analogue). Wayland: re-stage every frame because input region is double-buffered surface state. X11: one-shot via `XShape ShapeInput`. Isolated TU because `<X11/Xlib.h>` typedefs `KeyCode`/`Window`/`Status` into the global namespace. | Working. |

