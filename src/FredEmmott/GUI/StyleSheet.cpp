// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StyleSheet.hpp"

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <span>

namespace FredEmmott::GUI {

static bool StyleSelectorTokenMatches(
  StyleSelectorToken token,
  const Widgets::Widget* const widget) {
  if (const auto it = get_if<StyleClass>(&token)) {
    return widget->HasStyleClass(*it);
  }
  if (const auto it = get_if<const Widgets::Widget*>(&token)) {
    return widget == *it;
  }
  __debugbreak();
}

static bool StyleSelectorMatches(
  std::span<const StyleSelectorComponent> selector,
  std::span<const Widgets::Widget* const> widgetTrace,
  bool recursing = false) {
  if (selector.empty()) {
    return true;
  }
  if (widgetTrace.empty()) {
    return false;
  }

  auto w = widgetTrace.back();

  std::size_t consumed = 0;
  while (consumed < selector.size()) {
    auto [combinator, token] = selector[selector.size() - (++consumed)];
    if (!StyleSelectorTokenMatches(token, w)) {
      consumed = 0;
      break;
    }

    using enum StyleSelectorCombinator;
    switch (combinator) {
      case NestingSelector:
        continue;
      case DescendantSelector:
        break;
    }
  }

  if (consumed == 0 && !recursing) {
    return false;
  }

  const auto nextSelector = selector.subspan(0, selector.size() - consumed);
  auto nextWidgets
    = widgetTrace.subspan(0, widgetTrace.size() - (consumed ? 1 : 0));

  return StyleSelectorMatches(nextSelector, nextWidgets, true);
}

bool StyleSelectorMatches(
  const StyleSelector& selector,
  const std::vector<const Widgets::Widget*>& widgetTrace) {
  if (selector.empty()) {
    return false;
  }
  if (widgetTrace.empty()) {
    return false;
  }
  return StyleSelectorMatches(std::span {selector}, std::span {widgetTrace});
}
}// namespace FredEmmott::GUI