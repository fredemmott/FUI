// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "AcrylicBrush.hpp"
#include "Brush.hpp"
#include "StaticTheme/Resource.hpp"

namespace FredEmmott::GUI::Win32::Mica {
enum class Kind {
  Mica,
  MicaAlt,
};
template <Kind T>
struct WindowBackdrop {
  [[nodiscard]]
  static constexpr Kind GetKind() noexcept {
    return T;
  }

  constexpr bool operator==(const WindowBackdrop&) const noexcept = default;
};

}// namespace FredEmmott::GUI::Win32::Mica

namespace FredEmmott::GUI::Win32::Acrylic {
enum class Kind {
  Desktop,
  InApp,
};
template <Kind T>
struct WindowBackdrop {
 private:
  struct force_any_brush_t {};

 public:
  static constexpr force_any_brush_t force_any_brush {};
  WindowBackdrop() = delete;
  explicit constexpr WindowBackdrop(const AcrylicBrush& brush)
    : mBrush(brush) {}
  constexpr WindowBackdrop(force_any_brush_t, const Brush& brush)
    : mBrush(brush) {}

  explicit constexpr WindowBackdrop(
    const StaticTheme::Resource<Brush>* resource)
    : mBrush(resource) {}

  [[nodiscard]]
  static constexpr Kind GetKind() noexcept {
    return T;
  }

  [[nodiscard]]
  constexpr Brush GetBrush() const noexcept {
    return std::visit(
      felly::overload {
        [](const Brush& b) { return b; },
        [](const StaticTheme::Resource<Brush>* r) { return r->Resolve(); }},
      mBrush);
  }

  constexpr bool operator==(const WindowBackdrop&) const noexcept = default;

 private:
  std::variant<Brush, const StaticTheme::Resource<Brush>*> mBrush;
};

}// namespace FredEmmott::GUI::Win32::Acrylic

namespace FredEmmott::GUI::WindowBackdrops {
struct Transparent {
  constexpr bool operator==(const Transparent&) const = default;
};

using Win32_Mica
  = GUI::Win32::Mica::WindowBackdrop<GUI::Win32::Mica::Kind::Mica>;
using Win32_MicaAlt
  = GUI::Win32::Mica::WindowBackdrop<GUI::Win32::Mica::Kind::MicaAlt>;
using AppWindow = Win32_Mica;

using Win32_DesktopAcrylic
  = GUI::Win32::Acrylic::WindowBackdrop<GUI::Win32::Acrylic::Kind::Desktop>;
using Win32_InAppAcrylic
  = GUI::Win32::Acrylic::WindowBackdrop<GUI::Win32::Acrylic::Kind::InApp>;

}// namespace FredEmmott::GUI::WindowBackdrops

namespace FredEmmott::GUI {
using WindowBackdrop = std::variant<
  WindowBackdrops::Transparent,
  WindowBackdrops::AppWindow,
  // WindowBackdrops::Win32_Mica = AppWindow
  WindowBackdrops::Win32_MicaAlt,
  WindowBackdrops::Win32_DesktopAcrylic,
  WindowBackdrops::Win32_InAppAcrylic>;
}