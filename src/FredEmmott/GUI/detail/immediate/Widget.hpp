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
  static void Begin(const Widgets::WidgetStyles& styles, MangledID id) {
    auto& frame = tStack.back();
    auto pending = std::ranges::find(frame.mPending, id, &Widget::GetID);

    if (pending == frame.mPending.end()) {
      frame.mNewSiblings.push_back(new T(id, TFixedArgs...));
    } else {
      frame.mNewSiblings.push_back(*pending);
      frame.mPending.erase(pending);
    }

    auto it = GetCurrentNode();
    it->SetExplicitStyles(styles);

    tStack.emplace_back(it->GetChildren() | std::ranges::to<std::vector>());
    tStack.back().mNewSiblings.reserve(tStack.back().mPending.size());
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

  GetCurrentNode()->SetChildren(back.mNewSiblings);
  ++tStack.back().mNextIndex;
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail