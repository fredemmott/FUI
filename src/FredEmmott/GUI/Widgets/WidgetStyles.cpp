// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "WidgetStyles.hpp"

namespace FredEmmott::GUI::Widgets {

WidgetStyles WidgetStyles::InheritableStyles() const noexcept {
  return {
    .mBase = mBase.InheritableValues(),
    .mHover = mHover.InheritableValues(),
    .mActive = mActive.InheritableValues(),
  };
}
WidgetStyles& WidgetStyles::operator+=(const WidgetStyles& rhs) {
  mBase += rhs.mBase;
  mHover += rhs.mHover;
  mActive += rhs.mActive;
  return *this;
}

}// namespace FredEmmott::GUI::Widgets