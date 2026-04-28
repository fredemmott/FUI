// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Tooltip popups need to be mouse-passthrough so a user can drag the parent
// widget (e.g. a slider thumb) without the tooltip stealing the cursor. Win32
// gets this via WS_EX_TRANSPARENT; on Linux there's no SDL3-level equivalent,
// so we set an empty input region on the underlying surface directly.
//
// Lives in its own translation unit because <X11/Xlib.h> typedefs `KeyCode`,
// `Window`, `Status`, etc. into the global namespace and clashes with FUI's
// types if included alongside the rest of the SdlWindow code.
//
// Implementation strategy:
//
// * Wayland: the input region is double-buffered surface state, so a change
//   to set_input_region only takes effect on the next wl_surface.commit.
//   SDL3 owns the commit cycle (one per rendered frame), so we just stage
//   the empty region and let SDL3's next commit deliver it. We re-apply on
//   every frame in case SDL3 resets the region during its own setup, and we
//   never call wl_surface_commit ourselves — committing without a buffer
//   from an unexpected thread can confuse SDL3's frame submission.
//
// * X11: XShape ShapeInput is one-shot — once set, the X server keeps it.
//   So we apply once at popup creation.

#include <FredEmmott/GUI/detail/sdl_detail/PopupInputPassthrough.hpp>

#include <SDL3/SDL.h>
#include <wayland-client.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace FredEmmott::GUI::sdl_detail {
namespace {

enum class Backend { Unknown, Wayland, X11, Other };

Backend DetectBackend() {
  const auto* name = SDL_GetCurrentVideoDriver();
  if (!name) {
    return Backend::Unknown;
  }
  const std::string_view driver {name};
  if (driver == "wayland") {
    return Backend::Wayland;
  }
  if (driver == "x11") {
    return Backend::X11;
  }
  return Backend::Other;
}

// Wayland: cached compositor + empty region, bound once.
struct WaylandState {
  wl_compositor* mCompositor = nullptr;
  wl_region* mEmptyRegion = nullptr;
  bool mTried = false;
};
WaylandState& GetWaylandState() {
  static WaylandState s;
  return s;
}

void EnsureWaylandState(wl_display* display) {
  auto& state = GetWaylandState();
  if (state.mTried) {
    return;
  }
  state.mTried = true;

  auto* registry = wl_display_get_registry(display);
  if (!registry) {
    std::fprintf(
      stderr, "[FUI] tooltip passthrough: wl_display_get_registry failed\n");
    return;
  }
  static const wl_registry_listener kListener = {
    .global =
      [](
        void* data,
        wl_registry* reg,
        uint32_t name,
        const char* iface,
        uint32_t version) {
        if (std::strcmp(iface, wl_compositor_interface.name) == 0) {
          auto* s = static_cast<WaylandState*>(data);
          s->mCompositor = static_cast<wl_compositor*>(wl_registry_bind(
            reg,
            name,
            &wl_compositor_interface,
            std::min<uint32_t>(version, 4)));
        }
      },
    .global_remove = [](void*, wl_registry*, uint32_t) {},
  };
  wl_registry_add_listener(registry, &kListener, &state);
  wl_display_roundtrip(display);
  wl_registry_destroy(registry);

  if (!state.mCompositor) {
    std::fprintf(
      stderr, "[FUI] tooltip passthrough: wl_compositor not advertised\n");
    return;
  }
  state.mEmptyRegion = wl_compositor_create_region(state.mCompositor);
  std::fprintf(
    stderr, "[FUI] tooltip passthrough: Wayland empty input region ready\n");
}

void ApplyWaylandPassthrough(SDL_Window* window) {
  const auto props = SDL_GetWindowProperties(window);
  auto* surface = static_cast<wl_surface*>(SDL_GetPointerProperty(
    props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));
  auto* display = static_cast<wl_display*>(SDL_GetPointerProperty(
    props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
  if (!surface || !display) {
    return;
  }
  EnsureWaylandState(display);
  auto& state = GetWaylandState();
  if (!state.mEmptyRegion) {
    return;
  }
  // Stage the empty input region; SDL3's next frame commit will deliver it.
  // The region is double-buffered so re-staging per frame is cheap and
  // protects against SDL3 resetting the region during its own surface setup.
  wl_surface_set_input_region(surface, state.mEmptyRegion);
}

void ApplyX11Passthrough(SDL_Window* window) {
  const auto props = SDL_GetWindowProperties(window);
  auto* display = static_cast<Display*>(SDL_GetPointerProperty(
    props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
  const auto xwindow = static_cast<::Window>(
    SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
  if (!display || !xwindow) {
    std::fprintf(
      stderr, "[FUI] tooltip passthrough: X11 display/window unavailable\n");
    return;
  }
  XShapeCombineRectangles(
    display, xwindow, ShapeInput, 0, 0, nullptr, 0, ShapeSet, YXBanded);
}

}// namespace

void MakePopupInputPassthrough(SDL_Window* window) {
  switch (DetectBackend()) {
    case Backend::Wayland:
      ApplyWaylandPassthrough(window);
      return;
    case Backend::X11:
      ApplyX11Passthrough(window);
      return;
    case Backend::Other:
      std::fprintf(
        stderr,
        "[FUI] tooltip passthrough: unsupported video driver \"%s\"\n",
        SDL_GetCurrentVideoDriver());
      return;
    case Backend::Unknown:
      return;
  }
}

void RestakeTooltipInputRegion(SDL_Window* window) {
  // X11 ShapeInput is sticky; only the Wayland surface needs re-staging.
  if (DetectBackend() != Backend::Wayland) {
    return;
  }
  ApplyWaylandPassthrough(window);
}

}// namespace FredEmmott::GUI::Linux
