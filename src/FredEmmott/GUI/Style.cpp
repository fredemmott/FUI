// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#include <FredEmmott/GUI/StaticTheme/Generic.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <mutex>

#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

Style& Style::operator+=(const Style& other) {
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

  if (mAnd.empty()) {
    mAnd = other.mAnd;
  } else {
    for (auto&& [selector, style]: other.mAnd) {
      if (holds_alternative<std::monostate>(selector)) {
        *this += style;
        continue;
      }
      auto it = std::ranges::find(
        mAnd, selector, [](const auto& tuple) -> const auto& {
          return std::get<0>(tuple);
        });
      if (it != mAnd.end()) {
        auto buf = std::move(std::get<1>(*it));
        mAnd.erase(it);
        mAnd.emplace_back(selector, std::move(buf) + style);
      } else {
        mAnd.emplace_back(selector, style);
      }
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
        if (dup.mPriority != StylePropertyPriority::Important) {
          dup.mPriority = StylePropertyPriority::Inherited;
        }
        dest.insert_or_assign(key, dup);
        break;
      }
      case StylePropertyScope::SelfAndDescendants: {
        auto dup = value;
        if (dup.mPriority != StylePropertyPriority::Important) {
          dup.mPriority = StylePropertyPriority::Inherited;
        }
        dest.insert_or_assign(key, dup);
        break;
      }
    }
  }
}

const Style& Style::Empty() {
  static const Style sEmpty {};
  return sEmpty;
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
    auto it = std::ranges::find(
      ret.mAnd, selector, [](const auto& tuple) -> const auto& {
        return std::get<0>(tuple);
      });
    if (it == ret.mAnd.end()) {
      ret.mAnd.erase(it);
    }
    ret.mAnd.emplace_back(selector, dup);
  }
  return ret;
}

Style Style::BuiltinBaseline() {
  auto ret = StaticTheme::Generic::BodyTextBlockStyle()
    + Style()
        .Color(StaticTheme::TextFillColorPrimaryBrush)
        .Display(YGDisplayFlex)
        .FlexGrow(0)
        .FlexShrink(0)
        .Opacity(1)
        .Position(YGPositionTypeRelative)
        .ScaleX(1)
        .ScaleY(1)
        .TextAlign(TextAlign::Left)
        .TranslateX(0)
        .TranslateY(0)
        .And(
          PseudoClasses::Disabled,
          Style().Color(StaticTheme::TextFillColorDisabledBrush))
        .And(
          PseudoClasses::FocusVisible,
          Style()
            .OutlineRadius(StaticTheme::ControlCornerRadius)
            .OutlineColor(
              StaticTheme::Common::SystemControlFocusVisualPrimaryBrush)
            // FocusVisualMargin
            .OutlineOffset(3)
            // FocusVisualPrimaryThickness/DefaultFocusVisualPrimaryThickness
            .OutlineWidth(2));
  const auto makeBaseline = [](auto& prop) {
    prop.mScope = StylePropertyScope::Self;
    prop.mPriority = StylePropertyPriority::UserAgentBaseline;
  };
  for (auto&& [key, value]: ret.mStorage) {
    VisitStyleProperty(key, makeBaseline, value);
  }
  for (auto&& [selector, styles]: ret.mAnd) {
    for (auto&& [key, value]: styles.mStorage) {
      VisitStyleProperty(key, makeBaseline, value);
    }
  }
  return ret;
}

}// namespace FredEmmott::GUI