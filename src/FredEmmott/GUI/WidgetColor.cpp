// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "WidgetColor.hpp"

#include "Color.hpp"
#include "SystemColor.hpp"

namespace FredEmmott::GUI::WidgetColor {

Color Resolve(const Usage usage) noexcept {
  using namespace gui_detail::WinUI3Themes;
  switch (usage) {
#define DEFINE_CASE(X) \
  case Usage::X: \
    return DefaultTheme::X;
    FUI_WINUI_THEME_COLORS(DEFINE_CASE)
#undef DEFINE_CASE
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::WidgetColor