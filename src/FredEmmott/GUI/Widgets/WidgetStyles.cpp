// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "WidgetStyles.hpp"

namespace FredEmmott::GUI::Widgets {

WidgetStyles WidgetStyles::InheritableStyles() const noexcept {
  return {
    .mDefault = mDefault.InheritableValues(),
    .mHover = mHover.InheritableValues(),
  };
}
WidgetStyles& WidgetStyles::operator+=(const WidgetStyles& rhs) {
  mDefault += rhs.mDefault;
  mHover += rhs.mHover;
  return *this;
}

}// namespace FredEmmott::GUI::Widgets