// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <bit>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace FredEmmott::GUI {

class StyleClass {
  friend struct std::hash<StyleClass>;

 public:
  StyleClass() = delete;
  StyleClass(const StyleClass&) = default;
  StyleClass(StyleClass&&) = default;
  StyleClass& operator=(const StyleClass&) = default;
  StyleClass& operator=(StyleClass&&) = default;

  static StyleClass Make(std::string_view name);

  bool operator==(const StyleClass&) const noexcept = default;

 private:
  explicit StyleClass(std::string_view id) : mID(id) {}
  std::string_view mID {};
};
using StyleClasses = std::unordered_set<StyleClass>;
StyleClasses operator+(const StyleClasses&, StyleClass);
StyleClasses& operator+=(StyleClasses&, StyleClass);

}// namespace FredEmmott::GUI

template <>
struct std::hash<FredEmmott::GUI::StyleClass> {
  std::size_t operator()(const FredEmmott::GUI::StyleClass& c) const noexcept {
    return std::hash<uintptr_t> {}(std::bit_cast<uintptr_t>(c.mID.data()));
  }
};
