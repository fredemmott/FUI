// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <stack>

namespace FredEmmott::GUI::Immediate::immediate_detail {

namespace {

class ExplicitParentStack {
 public:
  void Push(Widgets::Widget* parent) {
    mStack.push(parent);
  }
  void Pop(const Widgets::Widget* const parent) {
    FUI_ALWAYS_ASSERT(mStack.top() == parent);
    mStack.pop();
  }

 private:
  std::stack<Widgets::Widget*> mStack;
};
class ExplicitParentStackDepth {
 public:
  void Push(Widgets::Widget*) {
    ++mDepth;
  }
  void Pop(const Widgets::Widget* const) {
    FUI_ALWAYS_ASSERT(mDepth > 0);
    --mDepth;
  }

 private:
  std::size_t mDepth {};
};

thread_local std::
  conditional_t<Config::Debug, ExplicitParentStack, ExplicitParentStackDepth>
    tExplicitParents;

}// namespace

void PushParentOverride(Widgets::Widget* parent) {
  tExplicitParents.Push(parent);
  tStack.push_back({.mNewSiblings = {parent}});
  tStack.push_back({.mPending = parent->GetLogicalChildren()});
}

void PopParentOverride() {
  FUI_ASSERT(tStack.size() >= 2);
  const auto parent = tStack.at(tStack.size() - 2).mNewSiblings.back();
  tExplicitParents.Pop(parent);
  parent->SetLogicalChildren(tStack.back().mNewSiblings);
  tStack.pop_back();
  FUI_ASSERT(!tStack.empty());
  FUI_ASSERT(tStack.back().mNewSiblings.size() == 1);
  FUI_ASSERT(tStack.back().mNewSiblings.back() == parent);
  tStack.pop_back();
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail