// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/concepts.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

namespace FredEmmott::GUI::Immediate::immediate_detail {

template <widget T, auto... TFixedArgs>
struct BeginWidget {
  void operator()(
    const Widgets::WidgetStyles& styles = {},
    MangledID id = MakeID<T>(tStack.back().mNextIndex)) const {
    TruncateUnlessNextIdEquals(id);

    auto& [siblings, i] = tStack.back();
    if (i == siblings.size()) {
      siblings.push_back(new T(id, TFixedArgs...));
    }

    auto it = GetCurrentNode();
    it->SetExplicitStyles(styles);

    tStack.emplace_back(it->GetChildren() | std::ranges::to<std::vector>());
  }
};

void EndWidget();

}// namespace FredEmmott::GUI::Immediate::immediate_detail