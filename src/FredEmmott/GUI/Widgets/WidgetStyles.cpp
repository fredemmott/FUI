// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "WidgetStyles.hpp"

namespace FredEmmott::GUI::Widgets {

WidgetStyles WidgetStyles::InheritableStyles() const noexcept {
  return {
    .mBase = mBase.InheritableValues(),
  };
}
WidgetStyles& WidgetStyles::operator+=(const WidgetStyles& rhs) {
  mBase += rhs.mBase;
  return *this;
}

}// namespace FredEmmott::GUI::Widgets