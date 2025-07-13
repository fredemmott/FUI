// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#include <FredEmmott/GUI/StaticTheme/Generic.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <mutex>

#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

Style& Style::operator+=(const Style& other) noexcept {
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
  this->m##X##Left##Y = m##X##Y + m##X##Left##Y + other.m##X##Y; \
  this->m##X##Top##Y = m##X##Y + m##X##Top##Y + other.m##X##Y; \
  this->m##X##Right##Y = m##X##Y + m##X##Right##Y + other.m##X##Y; \
  this->m##X##Bottom##Y = m##X##Y + m##X##Bottom##Y + other.m##X##Y;
  FUI_STYLE_EDGE_PROPERTIES(MERGE_EDGES)
#undef MERGE_EDGES

#define MERGE_PROPERTY(X, ...) this->m##X += other.m##X;
  FUI_ENUM_STYLE_PROPERTIES(MERGE_PROPERTY)
#undef MERGE_PROPERTIES
#define UNSET_ALL_EDGES(X, Y) this->m##X##Y = std::nullopt;
  FUI_STYLE_EDGE_PROPERTIES(UNSET_ALL_EDGES)
#undef UNSET_ALL_EDGES
  // We originally used `Vector::append_range()` here; profiling showed *a lot*
  // of time spend on allocations and deallocations, which this approach fixes.
  //
  // This is effectively using the tuple-of-vectors as a map; migrating to
  // `std::unordered_map()` is fairly trivial (as of 2025-07), but creates
  // its own performance issues, as it is not `constexpr` - so, no style
  // declarations can be constexpr either, also leading to excessive heap
  // allocations.
  //
  // If you change this, run a profiler while scrolling up and down rapidly with
  // the mouse.
  static_assert(
    Config::CompilerChecks::MinimumCPlusPlus < 202600
      || !Config::LibraryDeveloper,
    "Consider migrating `Style::mAnd` to `std::unordered_map` (P3372)");
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

  return *this;
}

Style Style::InheritableValues() const noexcept {
  Style ret;
  const auto copyIfInheritable = [this, &ret](auto member) {
    const auto& rhs = this->*member;
    if (!rhs.has_value()) {
      return;
    }
    auto& lhs = ret.*member;

    using enum StylePropertyScope;
    switch (rhs.mScope) {
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
#define COPY_IF_INHERITABLE(X, ...) copyIfInheritable(&Style::m##X);
  FUI_ENUM_STYLE_PROPERTIES(COPY_IF_INHERITABLE)
#undef COPY_IF_INHERITABLE
#define MAKE_INHERITABLE(X, ...) \
  X.mScope = StylePropertyScope::SelfAndDescendants;
#define COPY_AS_INHERITABLE(X, ...) \
  ret.m##X = rhs.m##X; \
  MAKE_INHERITABLE(ret.m##X)
  for (auto&& [selector, rhs]: mDescendants) {
    if (holds_alternative<std::monostate>(selector)) {
      FUI_ENUM_STYLE_PROPERTIES(COPY_AS_INHERITABLE);
      continue;
    }
    auto& it = ret.mAnd[selector];
    it += rhs;
#define MAKE_IT_INHERITABLE(X, ...) MAKE_INHERITABLE(it.m##X)
    FUI_ENUM_STYLE_PROPERTIES(MAKE_IT_INHERITABLE)
#undef MAKE_IT_INHERITABLE
  }
#undef COPY_AS_INHERITABLE
#undef MAKE_INHERITABLE
  return ret;
}

Style Style::BuiltinBaseline() {
  auto ret = StaticTheme::Generic::BodyTextBlockStyle + Style {
    .mColor = StaticTheme::TextFillColorPrimaryBrush,
    .mAnd = {
      { PseudoClasses::Disabled, Style {
        .mColor = StaticTheme::TextFillColorDisabledBrush,
      }},
    },
  };
#define PREVENT_INHERITANCE(X, ...) ret.m##X.mScope = StylePropertyScope::Self;
  FUI_ENUM_STYLE_PROPERTIES(PREVENT_INHERITANCE)
#undef PREVENT_INHERITANCE
  for (auto& [selector, style]: ret.mAnd) {
#define PREVENT_INHERITANCE(X, ...) \
  style.m##X.mScope = StylePropertyScope::Self;
    FUI_ENUM_STYLE_PROPERTIES(PREVENT_INHERITANCE)
    FUI_ASSERT(style.mAnd.empty());
#undef PREVENT_INHERITANCE
  }
  return ret;
}

}// namespace FredEmmott::GUI