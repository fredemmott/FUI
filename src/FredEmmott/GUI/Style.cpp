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

#define COPY_STORAGE_VALUES(TYPE, NAME) \
  if (m##NAME##Storage.empty()) { \
    m##NAME##Storage = other.m##NAME##Storage; \
  } else { \
    for (auto&& [key, value]: other.m##NAME##Storage) { \
      if (m##NAME##Storage.contains(key)) { \
        m##NAME##Storage[key] += value; \
      } else { \
        m##NAME##Storage.emplace(key, value); \
      } \
    } \
  }
  FUI_ENUM_STYLE_PROPERTY_TYPES(COPY_STORAGE_VALUES)
#undef COPY_STORAGE_VALUES
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

template <class T>
void Style::CopyInheritableValues(
  utility::unordered_map<style_detail::StyleProperty, StyleProperty<T>> dest,
  const utility::unordered_map<style_detail::StyleProperty, StyleProperty<T>>&
    source) {
  for (auto&& [key, value]: source) {
    if (!value.has_value()) {
      continue;
    }
    const auto scope = value.mScope.has_value()
      ? value.mScope.value()
      : style_detail::GetDefaultPropertyScope(key);
    switch (scope) {
      case StylePropertyScope::Self:
        break;
      case StylePropertyScope::SelfAndChildren: {
        auto dup = value;
        dup.mScope = StylePropertyScope::Self;
        dest.insert_or_assign(key, dup);
        break;
      }
      case StylePropertyScope::SelfAndDescendants:
        dest.insert_or_assign(key, value);
        break;
    }
  }
}

Style Style::InheritableValues() const noexcept {
  Style ret;
#define COPY_STORAGE(TYPE, NAME) \
  CopyInheritableValues(ret.m##NAME##Storage, m##NAME##Storage);
  FUI_ENUM_STYLE_PROPERTY_TYPES(COPY_STORAGE)
#undef COPY_STORAGE
  for (auto&& [selector, rhs]: mDescendants) {
    if (holds_alternative<std::monostate>(selector)) {
      ret += rhs;
      continue;
    }
    auto dup = rhs;
#define MAKE_INHERITABLE(TYPE, NAME) \
  for (auto&& [key, value]: dup.m##NAME##Storage) { \
    value.mScope = StylePropertyScope::SelfAndDescendants; \
  }
    FUI_ENUM_STYLE_PROPERTY_TYPES(MAKE_INHERITABLE)
#undef MAKE_INHERITABLE
    ret.mAnd[selector] += dup;
  }
  return ret;
}

Style Style::BuiltinBaseline() {
  auto ret = StaticTheme::Generic::BodyTextBlockStyle
    + Style()
        .Color(StaticTheme::TextFillColorPrimaryBrush)
        .And(
          PseudoClasses::Disabled,
          Style().Color(StaticTheme::TextFillColorDisabledBrush));
#define PREVENT_INHERITANCE(TYPE, NAME) \
  for (auto&& [_, value]: ret.m##NAME##Storage) { \
    value.mScope = StylePropertyScope::Self; \
  }
  FUI_ENUM_STYLE_PROPERTY_TYPES(PREVENT_INHERITANCE)
#undef PREVENT_INHERITANCE
  return ret;
}

}// namespace FredEmmott::GUI