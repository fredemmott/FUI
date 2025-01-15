// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::Widgets {

struct WidgetStyles {
  Style mBase;

  [[nodiscard]] WidgetStyles InheritableStyles() const noexcept;

  WidgetStyles& operator+=(const WidgetStyles& other);
  WidgetStyles operator+(const WidgetStyles& other) const {
    WidgetStyles ret = *this;
    ret += other;
    return ret;
  }

  bool operator==(const WidgetStyles&) const noexcept = default;
};

}// namespace FredEmmott::GUI::Widgets