// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#include <FredEmmott/GUI/assert.hpp>
#include <mutex>

#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

Style::Class Style::Class::Make(std::string_view name) {
  static std::mutex sClassMutex;
  static std::vector<std::string> sClassNames;

  std::unique_lock lock(sClassMutex);
  const auto it = std::ranges::find(sClassNames, name);
  if (it != sClassNames.end()) {
    return {static_cast<std::size_t>(it - std::ranges::begin(sClassNames))};
  }
  sClassNames.push_back(std::string(name));
  const auto id = static_cast<std::size_t>(sClassNames.size() - 1);
  return {id};
}

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
  mAnd.append_range(other.mAnd);
  mDescendants.append_range(other.mDescendants);

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
  ret.mDescendants = mDescendants;
  return ret;
}

Style Style::BuiltinBaseline() {
  Style ret {
    .mColor = StaticTheme::TextFillColorPrimaryBrush,
    .mFont = SystemFont::Body,
    .mAnd = {
      { Style::PseudoClass::Disabled, Style {
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
    FUI_ASSERT(style.mDescendants.empty());
#undef PREVENT_INHERITANCE
  }
  for (auto& [selector, style]: ret.mDescendants) {
#define PREVENT_INHERITANCE(X) style.m##X.mScope = StylePropertyScope::Self;
    FUI_STYLE_PROPERTIES(PREVENT_INHERITANCE)
    FUI_ASSERT(style.mAnd.empty());
    FUI_ASSERT(style.mDescendants.empty());
#undef PREVENT_INHERITANCE
  }
  return ret;
}

Style::ClassList operator+(const Style::ClassList& lhs, Style::Class rhs) {
  auto ret = lhs;
  ret.emplace(rhs);
  return ret;
}

}// namespace FredEmmott::GUI