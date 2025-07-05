// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/ID.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

namespace FredEmmott::GUI::Immediate::immediate_detail {

template <std::derived_from<Widgets::Widget> T, auto... TFixedArgs>
T* BeginWidget(const ID id) {
  auto& frame = tStack.back();

  if constexpr (Config::Debug) {
    if (std::ranges::contains(
          frame.mNewSiblings, id.GetValue(), &Widget::GetID)) {
      throw std::logic_error("All siblings must have different IDs");
    }
  }

  const auto pending
    = std::ranges::find(frame.mPending, id.GetValue(), &Widget::GetID);

  if (pending == frame.mPending.end()) {
    frame.mNewSiblings.push_back(new T(id.GetValue(), TFixedArgs...));
  } else {
    frame.mNewSiblings.push_back(*pending);
    frame.mPending.erase(pending);
  }

  const auto it = GetCurrentNode<T>();

  tStack.emplace_back(it->GetChildren() | std::ranges::to<std::vector>());
  tStack.back().mNewSiblings.reserve(tStack.back().mPending.size());
  return it;
}

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