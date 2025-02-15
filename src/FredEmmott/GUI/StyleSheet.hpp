// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <span>
#include <variant>
#include <vector>

#include "Style.hpp"
#include "StyleClass.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI {

enum class StyleSelectorCombinator {
  NestingSelector,// foo & bar { ... }
  DescendantSelector,// foo bar { ... },
};

using StyleSelectorToken = std::variant<StyleClass, const Widgets::Widget*>;
struct StyleSelectorComponent {
  StyleSelectorCombinator mCombinator;
  StyleSelectorToken mToken;

  StyleSelectorComponent() = delete;
  StyleSelectorComponent(StyleSelectorToken token)
    : mCombinator(StyleSelectorCombinator::DescendantSelector),
      mToken(token) {}
  StyleSelectorComponent(
    StyleSelectorCombinator combinator,
    StyleSelectorToken token)
    : mCombinator(combinator),
      mToken(token) {}
};

class StyleSelector {
 public:
  StyleSelector(StyleClass klass) {
    mComponents.emplace_back(klass);
  }
  StyleSelector(StyleSelectorComponent&& component) {
    mComponents.emplace_back(component);
  }
  StyleSelector(nullptr_t) = delete;
  explicit StyleSelector(const Widgets::Widget* const widget) {
    mComponents.emplace_back(widget);
  }

  explicit StyleSelector(const std::vector<StyleSelectorComponent>& components)
    : mComponents(components) {}

  operator std::span<const StyleSelectorComponent>() const noexcept {
    return std::span {mComponents};
  }

  void append(const StyleSelector& rhs) {
    mComponents.append_range(rhs.mComponents);
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return mComponents.empty();
  }

  void push_back(std::convertible_to<StyleSelectorToken> auto&& component) {
    mComponents.emplace_back(StyleSelectorToken {component});
  }

  void push_back(
    StyleSelectorCombinator combinator,
    std::convertible_to<StyleSelectorToken> auto&& component) {
    mComponents.emplace_back(combinator, component);
  }

  StyleSelector operator&(const StyleClass& other) const {
    auto ret = *this;
    ret.mComponents.emplace_back(
      StyleSelectorCombinator::NestingSelector, other);
    return ret;
  }

  StyleSelector operator,(const StyleSelector& other) const {
    auto ret = *this;
    ret.mComponents.reserve(mComponents.size() + other.mComponents.size());
    std::ranges::copy(other.mComponents, std::back_inserter(ret.mComponents));
    return ret;
  }

 private:
  std::vector<StyleSelectorComponent> mComponents;
};

using StyleSheet = std::vector<std::tuple<StyleSelector, Style>>;

[[nodiscard]]
bool StyleSelectorMatches(
  const StyleSelector& selector,
  const std::vector<const Widgets::Widget*>& widgetPath);

}// namespace FredEmmott::GUI