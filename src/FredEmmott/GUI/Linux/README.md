# Linux backend

Linux-specific implementations of FUI's platform abstractions, mirroring
[`Windows/`](../Windows/) one-to-one. Every file here corresponds to a
Windows counterpart that gates on `_WIN32`; the Linux file is selected
instead when building on Linux (see [`src/lib.cmake`](../../../lib.cmake)).

For *why* the major decisions were made (SDL3 vs raw Wayland, Vulkan vs
Vulkan+GL, etc.).

## Module map

| File | Provides | Status |
|---|---|---|
| [`LinuxWindow.{hpp,cpp}`](LinuxWindow.hpp) | Abstract `Window` base for Linux: SDL3 windowing, event pump, input dispatch, clipboard, cursor, IME hook, popup wiring. Owns everything that isn't render-backend-specific. | Working. |
| [`LinuxSkiaVulkanWindow.{hpp,cpp}`](LinuxSkiaVulkanWindow.hpp) | Concrete `LinuxWindow` subclass: Skia Ganesh on Vulkan. Owns `VkInstance`/`VkDevice`/swapchain, wraps swapchain images as `SkSurface`s, drives presentation. Parallel to Windows' `Win32Direct3D12GaneshWindow`. | Working. |
| [`Font.cpp`](Font.cpp) | `Font` ctor/`WithSize`/`WithWeight`/`MeasureTextWidth` — Skia-only path (no DirectWrite). Different from Windows in one place: an empty/fallback `SkFont` stays a real `SkFont` rather than collapsing to `monostate`, because fontconfig substitutes don't always exist for Segoe families. | Working. |
| [`SystemTheme.cpp`](SystemTheme.cpp) | `SystemTheme::Resolve` for sys-color and accent slots. Stub: hard-coded Win11 Light palette. | Stub. Real impl reads `xdg-desktop-portal` color-scheme + accent. |
| [`StaticTheme.cpp`](StaticTheme.cpp) | `StaticTheme::GetCurrent` and override stack. Stub: returns `Theme::Light`. | Stub. Real impl subscribes to portal Settings signals for live switching. |
| [`SystemSettings.cpp`](SystemSettings.cpp) | `SystemSettings::Get*` (caret blink, key repeat, scroll lines, animations). Stub: hard-coded values matching Windows factory defaults so widgets behave predictably. | Stub. Real impl reads portal Settings / gsettings. |
| [`IconProvider.cpp`](IconProvider.cpp) | `ApplicationIconProvider` impl. Stub: always reports invalid; `TitleBar` then renders no icon. | Stub. Real impl hooks freedesktop icon themes via Gio / xdg-icon-resource. |
| [`NumberBox.cpp`](NumberBox.cpp) | Immediate-mode `NumberBox(float*)` — minimum-viable two-way binding with `std::from_chars` parsing and `%g` formatting. Replaces the cross-platform [`Immediate/NumberBox.cpp`](../Immediate/NumberBox.cpp) which is `wchar_t`/`UChar`-coupled. Handles the focus-ping-pong and external-update edge cases inline. | MVP. Replace once cross-platform NumberBox is portable. |
| [`TextBox.cpp`](TextBox.cpp) | Widget-level `TextBox`: UTF-8 append on text-input events, backspace, click→focus, paint. Replaces the cross-platform [`Widgets/TextBox.cpp`](../Widgets/TextBox.cpp) which is Win32-TSF-coupled. | MVP. Selection / cursor positioning / IME preedit / undo-redo are not yet implemented. |

## Class hierarchy

```
                    Window  (cross-platform abstract base, all pure virtual)
                       │
                  LinuxWindow  (abstract — SDL3, input, popups; render methods unimplemented)
                       │
              LinuxSkiaVulkanWindow  (concrete — Vulkan + Skia Ganesh)
```

`LinuxWindow` is **abstract**: render-side methods (`InitializeGraphicsAPI`,
`GetFramePainter`, `CreatePopup`, `ResizeBackend`) are pure virtual, leaving
backend implementations no choice but to opt in explicitly. There is
intentionally no "null backend" — building without a renderer is not a
supported configuration.

`ResizeIfNeeded` is `final` on `LinuxWindow` and uses the template-method
pattern: the base orchestrates `popup-fit → ResizeBackend() → clear-flag`,
and each backend supplies `ResizeBackend()` as the swapchain-rebuild hook.
This is inverted from the more common "subclass overrides and chains to
base" — putting the base in charge guarantees popup-fit happens before the
backend re-queries the SDL pixel size.

Adding a new render backend (e.g. a future Wayland-direct or GL path) means:

1. New subclass under this directory (e.g. `LinuxSomethingWindow`).
2. Implement the four pure virtuals.
3. Override `GetSDLWindowFlags()` if the backend needs SDL flags beyond the
   resizable + high-DPI defaults the base supplies.

## Conventions

- Public headers must not include `<SDL3/SDL.h>` or `<vulkan/vulkan.h>`.
  `LinuxWindow.hpp` forward-declares `SDL_Window` / `SDL_Cursor` /
  `SDL_Event` at global scope so callers stay isolated; SDL types are
  smuggled through the header as `uint64_t` (e.g. `GetSDLWindowFlags`'s
  return type) where needed.

