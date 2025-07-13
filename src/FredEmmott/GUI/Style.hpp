// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>

#include <FredEmmott/utility/unordered_map.hpp>
#include <unordered_set>

#include "Brush.hpp"
#include "Font.hpp"
#include "PseudoClasses.hpp"
#include "StyleClass.hpp"
#include "StyleProperty.hpp"
#include "StylePropertyTypes.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI {

struct Style {
  using Selector
    = std::variant<std::monostate, StyleClass, const Widgets::Widget*>;

#define FUI_DECLARE_STYLE_PROPERTY(NAME, TYPE, ...) StyleProperty<TYPE> m##NAME;
  FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_STYLE_PROPERTY)
#undef FUI_DECLARE_STYLE_PROPERTIES

  utility::unordered_map<Selector, Style> mAnd;
  utility::unordered_map<Selector, Style> mDescendants;

  [[nodiscard]] Style InheritableValues() const noexcept;
  [[nodiscard]]
  static Style BuiltinBaseline();

  Style& operator+=(const Style& other) noexcept;

  bool operator==(const Style& other) const noexcept = default;
};

inline Style operator+(const Style& lhs, const Style& rhs) noexcept {
  Style ret {lhs};
  ret += rhs;
  return ret;
}

}// namespace FredEmmott::GUI
