// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#include <FredEmmott/GUI/StaticTheme/Generic.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <mutex>

#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

StyleProperties& StyleProperties::operator+=(
  const StyleProperties& other) noexcept {
  /* Set the lhs to the rhs, if the rhs is set.
   * e.g. with:
   *
   * ```css
   * myclass {
   *   color: white;
   *   background-color: black;
   * }
   * myclass:hover {
   *   background-color: red;
   * }
   * ```
   *
   * `myclass + myclass:hover` should result in:
   *
   *  ```
   *  {
   *    color: white;
   *    background-color: red;
   *  }
   *  ```
   */

  // other.m##X##Edge##Y will be added in MERGE_PROPERTY below
#define MERGE_EDGES(X, Y) \
  this->X##Left##Y() = X##Y() + X##Left##Y() + other.X##Y(); \
  this->X##Top##Y() = X##Y() + X##Top##Y() + other.X##Y(); \
  this->X##Right##Y() = X##Y() + X##Right##Y() + other.X##Y(); \
  this->X##Bottom##Y() = X##Y() + X##Bottom##Y() + other.X##Y();
  FUI_STYLE_EDGE_PROPERTIES(MERGE_EDGES)
#undef MERGE_EDGES

#define MERGE_PROPERTY(X, ...) \
  if (other.Has##X()) { \
    this->X() += other.X(); \
  }
  FUI_ENUM_STYLE_PROPERTIES(MERGE_PROPERTY)
#undef MERGE_PROPERTIES
#define UNSET_ALL_EDGES(X, Y) this->Unset##X##Y();
  FUI_STYLE_EDGE_PROPERTIES(UNSET_ALL_EDGES)
#undef UNSET_ALL_EDGES

  return *this;
}

Style& Style::operator+=(const Style& other) noexcept {
  StyleProperties::operator+=(other);

  if (mAnd.empty()) {
    mAnd = other.mAnd;
  } else {
    for (auto&& [selector, style]: other.mAnd) {
      if (holds_alternative<std::monostate>(selector)) {
        *this += style;
        continue;
      }
      mAnd[selector] += style;
    }
  }

  if (mDescendants.empty()) {
    mDescendants = other.mDescendants;
  } else {
    for (auto&& [selector, style]: other.mDescendants) {
      mDescendants[selector] += style;
    }
  }

  return *this;
}

Style Style::InheritableValues() const noexcept {
  Style ret;
  const auto copyIfInheritable
    = [this, &ret](auto member, const auto defaultScope) {
        const auto& rhs = std::invoke(member, *this);
        if (!rhs.has_value()) {
          return;
        }
        auto& lhs = std::invoke(member, ret);

        using enum StylePropertyScope;
        switch (rhs.mScope.value_or(defaultScope)) {
          case Self:
            return;
          case SelfAndChildren:
            lhs = rhs;
            lhs.mScope = Self;
            break;
          case SelfAndDescendants:
            lhs = rhs;
            break;
        }
      };
#define COPY_IF_INHERITABLE(X, ...) \
  copyIfInheritable( \
    [](auto&& style) -> auto& { return style.X(); }, \
    style_detail::default_property_scope_v<style_detail::StyleProperty::X>);
  FUI_ENUM_STYLE_PROPERTIES(COPY_IF_INHERITABLE)
#undef COPY_IF_INHERITABLE
#define MAKE_INHERITABLE(X, ...) \
  X.mScope = StylePropertyScope::SelfAndDescendants;
#define COPY_AS_INHERITABLE(X, ...) \
  ret.X() = rhs.X(); \
  MAKE_INHERITABLE(ret.X())
  for (auto&& [selector, rhs]: mDescendants) {
    if (holds_alternative<std::monostate>(selector)) {
      FUI_ENUM_STYLE_PROPERTIES(COPY_AS_INHERITABLE);
      continue;
    }
    auto& it = ret.mAnd[selector];
    it += rhs;
#define MAKE_IT_INHERITABLE(X, ...) MAKE_INHERITABLE(it.X())
    FUI_ENUM_STYLE_PROPERTIES(MAKE_IT_INHERITABLE)
#undef MAKE_IT_INHERITABLE
  }
#undef COPY_AS_INHERITABLE
#undef MAKE_INHERITABLE
  return ret;
}

Style Style::BuiltinBaseline() {
  auto ret = StaticTheme::Generic::BodyTextBlockStyle
    + Style()
        .Color(StaticTheme::TextFillColorPrimaryBrush)
        .And(
          PseudoClasses::Disabled,
          Style().Color(StaticTheme::TextFillColorDisabledBrush));
#define PREVENT_INHERITANCE(X, ...) ret.X().mScope = StylePropertyScope::Self;
  FUI_ENUM_STYLE_PROPERTIES(PREVENT_INHERITANCE)
#undef PREVENT_INHERITANCE
  for (auto&& [selector, style]: ret.mAnd) {
#define PREVENT_INHERITANCE(X, ...) style.X().mScope = StylePropertyScope::Self;
    FUI_ENUM_STYLE_PROPERTIES(PREVENT_INHERITANCE)
#undef PREVENT_INHERITANCE
  }
  return ret;
}

}// namespace FredEmmott::GUI