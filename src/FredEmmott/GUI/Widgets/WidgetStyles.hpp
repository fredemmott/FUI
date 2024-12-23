// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI {

struct WidgetStyles {
  Style mDefault;
  Style mHover;

  [[nodiscard]] WidgetStyles InheritableStyles() const noexcept;

  WidgetStyles& operator+=(const WidgetStyles& other);
  WidgetStyles operator+(const WidgetStyles& other) const {
    WidgetStyles ret = *this;
    ret += other;
    return ret;
  }
};

}// namespace FredEmmott::GUI