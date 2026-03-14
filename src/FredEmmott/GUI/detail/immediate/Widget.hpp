// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/ID.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

namespace FredEmmott::GUI::Immediate::immediate_detail {

static_assert(std::same_as<ID::value_type, Widgets::Widget::id_type>);

template <std::derived_from<Widgets::Widget> T, class... Args>
T* ChildlessWidget(const ID id, Args&&... args) {
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
    frame.mNewSiblings.push_back(
      new T(id.GetValue(), std::forward<Args>(args)...));
    frame.mNewSiblings.back()->ForceLogicalParent(GetCurrentParentNode());
  } else {
    frame.mNewSiblings.push_back(*pending);
    frame.mPending.erase(pending);
  }

  return static_cast<T*>(frame.mNewSiblings.back());
}

template <std::derived_from<Widgets::Widget> T, class... Args>
T* BeginWidget(const ID id, Args&&... args) {
  const auto it = ChildlessWidget<T>(id, std::forward<Args>(args)...);
  FUI_ASSERT(it == GetCurrentNode<T>());

  tStack.emplace_back(
    it->GetLogicalChildren() | std::ranges::to<std::vector>());
  tStack.back().mNewSiblings.reserve(tStack.back().mPending.size());
  return it;
}

template <class T = Widgets::Widget>
void EndWidget() {
  const auto parent = GetCurrentParentNode<T>();
  FUI_ASSERT(
    parent,
    "in EndWidget without a parent, or with a parent of a differing type");

  parent->SetLogicalChildren(tStack.back().mNewSiblings);
  tStack.pop_back();
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail