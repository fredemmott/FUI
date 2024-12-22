// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "WidgetFont.hpp"

#include "Font.hpp"
#include "SystemFont.hpp"
#include "detail/font_detail.hpp"

namespace FredEmmott::GUI::WidgetFont {

using namespace font_detail;

Font Resolve(const Usage usage) noexcept {
  switch (usage) {
    case ControlContent:
      return SystemFont::Body;
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::WidgetFont