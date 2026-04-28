# SDL3 windowing layer

Cross-platform-eligible windowing base built on SDL3, plus the Skia/Vulkan
renderer subclass currently used for Linux. Future macOS (Skia/Metal) and
SDL3-on-Windows (portability testing of Skia/D3D12 over SDL) backends would
slot in here as additional `SdlWindow` subclasses.

Linux-only shell integration (theme via xdg-desktop-portal, settings,
fontconfig wiring, Wayland/X11 input-region tweaks) lives in [`../Linux/`](../Linux/),
not here, and is selected by `__linux__` independent of which `SdlWindow`
subclass is in use.

## Module map

| File | Provides | Status |
|---|---|---|
| [`SdlWindow.{hpp,cpp}`](SdlWindow.hpp) | Abstract `Window` base over SDL3: event pump, input dispatch (mouse/keyboard/pen/wheel), clipboard, cursor, IME hook, popup wiring, DPI, resize. Owns everything that isn't render-backend-specific. | Working on Linux; not yet exercised on macOS / Windows. |
| [`SdlSkiaVulkanWindow.{hpp,cpp}`](SdlSkiaVulkanWindow.hpp) | Concrete `SdlWindow` subclass: Skia Ganesh on Vulkan. Owns `VkInstance`/`VkDevice`/swapchain, wraps swapchain images as `SkSurface`s, drives presentation. Parallel to Windows' `Win32Direct3D12GaneshWindow`. | Working. |

## Class hierarchy

```
                    Window  (cross-platform abstract base, all pure virtual)
                       │
                   SdlWindow  (abstract — SDL3, input, popups; render methods unimplemented)
                       │
              SdlSkiaVulkanWindow  (concrete — Vulkan + Skia Ganesh)
```

`SdlWindow` is **abstract**: render-side methods (`InitializeGraphicsAPI`,
`GetFramePainter`, `CreatePopup`, `ResizeBackend`) are pure virtual, leaving
backend implementations no choice but to opt in explicitly. There is
intentionally no "null backend" — building without a renderer is not a
supported configuration.

`ResizeIfNeeded` is `final` on `SdlWindow` and uses the template-method
pattern: the base orchestrates `popup-fit → ResizeBackend() → clear-flag`,
and each backend supplies `ResizeBackend()` as the swapchain-rebuild hook.
This is inverted from the more common "subclass overrides and chains to
base" — putting the base in charge guarantees popup-fit happens before the
backend re-queries the SDL pixel size.

Adding a new render backend (e.g. a Linux Skia+OpenGL path, a macOS
Skia+Metal path, an SDL3-hosted Skia+D3D12 path on Windows) means:

1. New subclass under this directory (e.g. `SdlSkiaMetalWindow`).
2. Implement the four pure virtuals.
3. Override `GetSDLWindowFlags()` if the backend needs SDL flags beyond the
   resizable + high-DPI defaults the base supplies.

## Conventions

- Public headers must not include `<SDL3/SDL.h>` or `<vulkan/vulkan.h>`.
  `SdlWindow.hpp` forward-declares `SDL_Window` / `SDL_Cursor` / `SDL_Event`
  at global scope so callers stay isolated; SDL types are smuggled through
  the header as `uint64_t` (e.g. `GetSDLWindowFlags`'s return type) where
  needed.
