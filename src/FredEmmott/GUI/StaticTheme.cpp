// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StaticTheme.hpp"

#include <FredEmmott/GUI/detail/WinUI3Themes/Macros.hpp>
#include <FredEmmott/GUI/detail/WinUI3Themes/Themes.hpp>

#include "Brush.hpp"
#include "Color.hpp"
#include "SystemColor.hpp"

namespace FredEmmott::GUI::StaticTheme {

Color Resolve(const ColorType color) noexcept {
  using namespace gui_detail::WinUI3Themes;
  switch (color) {
#define DEFINE_CASE(X) \
  case X: \
    return DefaultTheme.m##X;
    FUI_WINUI_THEME_COLORS(DEFINE_CASE)
#undef DEFINE_CASE
  }
  std::unreachable();
}

Brush Resolve(const BrushType brush) noexcept {
  using namespace gui_detail::WinUI3Themes;
  switch (brush) {
#define DEFINE_CASE(X) \
  case X: \
    return DefaultTheme.m##X;
    FUI_WINUI_THEME_BRUSHES(DEFINE_CASE)
#undef DEFINE_CASE
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::StaticTheme