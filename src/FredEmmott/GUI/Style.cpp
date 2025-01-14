// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#include <mutex>

namespace FredEmmott::GUI {

namespace {
std::mutex gClassMutex;
std::vector<std::string> gClassNames;
}// namespace

Style::Class Style::Class::Make(std::string_view name) {
  std::unique_lock lock(gClassMutex);
  const auto it = std::ranges::find(gClassNames, name);
  if (it != gClassNames.end()) {
    return {static_cast<std::size_t>(it - std::ranges::begin(gClassNames))};
  }
  gClassNames.push_back(std::string(name));
  return {static_cast<std::size_t>(gClassNames.size() - 1)};
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

Style::ClassList operator+(const Style::ClassList& lhs, Style::Class rhs) {
  auto ret = lhs;
  ret.emplace(rhs);
  return ret;
}

}// namespace FredEmmott::GUI