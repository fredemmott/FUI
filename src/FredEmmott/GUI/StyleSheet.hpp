// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

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

using StyleSelector = std::vector<StyleSelectorComponent>;
using StyleSheet = std::vector<std::tuple<StyleSelector, Style>>;

template <class... TArgs>
auto MakeSelector(TArgs&&... args) {
  return StyleSelector {StyleSelectorComponent {std::forward<TArgs>(args)}...};
}

[[nodiscard]]
bool StyleSelectorMatches(
  const StyleSelector& selector,
  const std::vector<const Widgets::Widget*>& widgetPath);

}// namespace FredEmmott::GUI