// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#include <FredEmmott/GUI/StaticTheme/Generic.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <mutex>

#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

StyleProperties& StyleProperties::operator+=(const StyleProperties& other) {
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

  if (mStorage.empty()) {
    mStorage = other.mStorage;
  } else {
    for (auto&& [key, value]: other.mStorage) {
      if (!mStorage.contains(key)) {
        mStorage.emplace(key, value);
        continue;
      }
      VisitStyleProperty(
        key,
        [](auto& dest, const auto& src) { dest += src; },
        mStorage[key],
        value);
    }
  }
#undef COPY_STORAGE_VALUES
#define UNSET_ALL_EDGES(X, Y) \
  mStorage.erase(StylePropertyKey::X##Y); \
  FUI_STYLE_EDGE_PROPERTIES(UNSET_ALL_EDGES)
#undef UNSET_ALL_EDGES

  return *this;
}

Style& Style::operator+=(const Style& other) {
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
  decltype(mStorage)& dest,
  const decltype(mStorage)& source) {
  for (auto&& [key, untypedValue]: source) {
    if (!holds_alternative<StyleProperty<T>>(untypedValue)) {
      continue;
    }
    const auto& value = get<StyleProperty<T>>(untypedValue);

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
  CopyInheritableValues<TYPE>(ret.mStorage, mStorage);
  FUI_ENUM_STYLE_PROPERTY_TYPES(COPY_STORAGE)
#undef COPY_STORAGE
  for (auto&& [selector, rhs]: mDescendants) {
    if (holds_alternative<std::monostate>(selector)) {
      ret += rhs;
      continue;
    }
    auto dup = rhs;
    for (auto&& [key, value]: dup.mStorage) {
      VisitStyleProperty(
        key,
        [](auto& prop) {
          prop.mScope = StylePropertyScope::SelfAndDescendants;
        },
        value);
    }
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
  for (auto&& [key, value]: ret.mStorage) {
    VisitStyleProperty(
      key, [](auto& prop) { prop.mScope = StylePropertyScope::Self; }, value);
  }
  return ret;
}

}// namespace FredEmmott::GUI