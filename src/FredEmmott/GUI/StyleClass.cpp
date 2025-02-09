// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StyleClass.hpp"

#include <FredEmmott/GUI/assert.hpp>
#include <mutex>

#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

StyleClass StyleClass::Make(std::string_view name) {
  static std::mutex sClassMutex;
  static std::list<std::string> sClassNames;

  std::unique_lock lock(sClassMutex);
  const auto it = std::ranges::find(sClassNames, name);
  if (it != sClassNames.end()) {
    return StyleClass {*it};
  }
  sClassNames.emplace_back(std::string {name});
  return StyleClass {sClassNames.back()};
}

StyleClasses& operator+=(StyleClasses& lhs, StyleClass rhs) {
  lhs.emplace(rhs);
  return lhs;
}

StyleClasses operator+(const StyleClasses& lhs, StyleClass rhs) {
  auto ret = lhs;
  ret += rhs;
  return ret;
}

}// namespace FredEmmott::GUI
