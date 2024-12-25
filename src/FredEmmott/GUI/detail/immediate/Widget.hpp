// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>
#include <FredEmmott/GUI/Immediate/concepts.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

namespace FredEmmott::GUI::Immediate::immediate_detail {

template <widget T, ActivatedFlag T::* P = nullptr, auto... TFixedArgs>
struct BeginWidget {
  static constexpr bool HasActivation = (P != nullptr);

  void operator()(
    const Widgets::WidgetStyles& styles = {},
    MangledID id = MakeID<T>(tStack.back().mNextIndex)) const
    requires(!HasActivation)
  {
    Begin(styles, id);
  }

  void operator()(
    bool* activated,
    const Widgets::WidgetStyles& styles = {},
    MangledID id = MakeID<T>(tStack.back().mNextIndex)) const
    requires(HasActivation)
  {
    Begin(styles, id);
    if (activated) {
      *activated = (GetCurrentParentNode<T>()->*P).TestAndClear();
    }
  }

 private:
  void Begin(const Widgets::WidgetStyles& styles, MangledID id) const {
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

template <class T>
void EndWidget() {
#ifndef NDEBUG
  if (!GetCurrentParentNode<T>()) [[unlikely]] {
    throw std::logic_error("EndWidget type does not match BeginWidget");
  }
#endif
  const auto back = std::move(tStack.back());
  tStack.pop_back();

  GetCurrentNode()->SetChildren(back.mChildren);
  ++tStack.back().mNextIndex;
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail