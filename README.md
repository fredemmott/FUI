# FredEmmott::GUI

This is a work-in-progress, visually recreating WinUI3 using Skia, and an API that feels similar to immediate-mode GUI libraries. This library is **not** a true immediate-mode GUI.

While it is capable of rendering to texture for integration in other rendering pipelines, it is primarily intended for GUI app development.

You shouldn't use this (yet?).

## Example

See [`src/main.cpp`](src/main.cpp):

![Demo](demo.png)

## Requirements

You shouldn't use this (yet?).

Only Windows is currently supported.

### Compiler requirements

- C++23
- source interpreted as UTF-8
  - on most compilers, this is the default
  - For MSVC, see [Microsoft's documentation](https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8)

FUI has `static_assert()s` for these requirements

### Additional Windows requirements

- DPI awareness should be set to `DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2`
- `RoInitialize()` or `winrt::init_aparment() must be called`; `CoInitializeEx()` *might* be sufficient, but is unsupported.
- Application must target Windows 10 or newer

The easiest way to meet these requirements is to:

- use `Win32Window::WinMain()` - this will automatically set up DPI awareness and call `RoInitialize()`, unless options are explicitly set to disable the behavior
- use a recent version of the Windows SDK
- make your build process set `-D_WIN32_WINNT=_WIN32_WINNT_WIN10` *and* `-DNTDDI_VERSION=NTDDI_WIN10`, or use the values for newer Windows versions.

The original Windows 10 version is currently targeted, but this is just because it's the oldest version that happens to provide all the features this library current uses. For my current needs, I only care about *consumer x64* versions of Windows 10 that *Microsoft currently support for all consumers* outside of extended service plans; if things work under any non-consumer versions - including LTSC - this is a happy accident. Future versions of this library may require a newer version of Windows, including v0.x and v1.x versions of this library.