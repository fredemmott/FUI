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
  const auto merge = [this, &other](auto member) {
    auto& lhs = this->*member;
    const auto& rhs = other.*member;

    lhs += rhs;
  };

#define MERGE_PROPERTY(X) merge(&Style::m##X);
  FUI_STYLE_PROPERTIES(MERGE_PROPERTY)
#undef MERGE_PROPERTIES
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
  if (!other.mAnd.empty()) {
    if (mAnd.empty()) {
      mAnd = other.mAnd;
    } else {
      for (auto&& [selector, style]: other.mAnd) {
        if (holds_alternative<std::monostate>(selector)) {
          *this += style;
          continue;
        }
        auto it = std::ranges::find(
          mAnd, selector, [](auto&& tuple) { return get<0>(tuple); });
        if (it == mAnd.end()) {
          mAnd.emplace_back(selector, style);
        } else {
          get<1>(*it) += style;
        }
      }
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
#define COPY_IF_INHERITABLE(X) copyIfInheritable(&Style::m##X);
  FUI_STYLE_PROPERTIES(COPY_IF_INHERITABLE)
#undef COPY_IF_INHERITABLE
#define MAKE_INHERITABLE(X) X.mScope = StylePropertyScope::SelfAndDescendants;
#define COPY_AS_INHERITABLE(X) \
  ret.m##X = rhs.m##X; \
  MAKE_INHERITABLE(ret.m##X)
  for (auto&& [selector, rhs]: mDescendants) {
    if (holds_alternative<std::monostate>(selector)) {
      FUI_STYLE_PROPERTIES(COPY_AS_INHERITABLE);
      continue;
    }
    ret.mAnd.emplace_back(selector, rhs);
    auto& it = std::get<1>(ret.mAnd.back());
#define MAKE_IT_INHERITABLE(X) MAKE_INHERITABLE(it.m##X)
    FUI_STYLE_PROPERTIES(MAKE_IT_INHERITABLE)
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
#define PREVENT_INHERITANCE(X) ret.m##X.mScope = StylePropertyScope::Self;
  FUI_STYLE_PROPERTIES(PREVENT_INHERITANCE)
#undef PREVENT_INHERITANCE
  for (auto& [selector, style]: ret.mAnd) {
#define PREVENT_INHERITANCE(X) style.m##X.mScope = StylePropertyScope::Self;
    FUI_STYLE_PROPERTIES(PREVENT_INHERITANCE)
    FUI_ASSERT(style.mAnd.empty());
#undef PREVENT_INHERITANCE
  }
  return ret;
}

}// namespace FredEmmott::GUI