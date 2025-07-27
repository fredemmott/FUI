// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FocusManager.hpp"

#include <stack>

#include "Widgets/Focusable.hpp"
#include "assert.hpp"

namespace FredEmmott::GUI {

namespace {
bool IsFocusable(Widgets::Widget const* widget) {
  if (widget->IsDisabled()) {
    return false;
  }
  return dynamic_cast<Widgets::IFocusable const*>(widget);
}

thread_local std::stack<FocusManager*> tInstances;
}// namespace

FocusManager* FocusManager::Get() {
  if (tInstances.empty()) {
    return nullptr;
  }
  return tInstances.top();
}

void FocusManager::PushInstance(FocusManager* it) {
  tInstances.push(it);
}

void FocusManager::PopInstance(FocusManager* it) {
  FUI_ASSERT(it == tInstances.top());
  tInstances.pop();
}

FocusManager::~FocusManager() = default;

FocusManager::FocusManager(Widgets::Widget* rootWidget)
  : mRootWidget(rootWidget) {}

std::optional<std::tuple<Widgets::Widget*, FocusKind>>
FocusManager::GetFocusedWidget() const {
  if (!mFocusedWidget) {
    return std::nullopt;
  }
  return std::tuple {mFocusedWidget, mFocusKind};
}

void FocusManager::GivePointerFocus(Widgets::Widget* widget) {
  mFocusedWidget = widget;
  mFocusKind = FocusKind::Pointer;
}

void FocusManager::FocusNextWidget() {
  mFocusKind = FocusKind::Keyboard;
  if (!mFocusedWidget) {
    this->FocusFirstWidget();
    return;
  }

  auto child = std::exchange(mFocusedWidget, nullptr);
  while (const auto parent = child->GetParent()) {
    const auto children = parent->GetChildren();
    const auto it = std::ranges::find(children, child);
    for (auto&& sibling: std::ranges::subrange(it + 1, children.end())) {
      if (const auto target = FirstFocusableWidget(sibling)) {
        mFocusedWidget = target;
        return;
      }
      if (parent == mRootWidget) {
        break;
      }
      if (IsFocusable(parent)) {
        mFocusedWidget = parent;
        return;
      }
    }
    child = parent;
  }
  FocusFirstWidget();
}

void FocusManager::FocusPreviousWidget() {
  mFocusKind = FocusKind::Keyboard;
  if (!mFocusedWidget) {
    this->FocusFirstWidget();
    return;
  }

  auto child = std::exchange(mFocusedWidget, nullptr);
  while (const auto parent = child->GetParent()) {
    const auto children = parent->GetChildren();
    const auto it = std::ranges::find(children, child);
    for (auto&& sibling:
         std::ranges::subrange(children.begin(), it) | std::views::reverse) {
      if (const auto target = FirstFocusableWidget(sibling)) {
        mFocusedWidget = target;
        return;
      }
    }
    if (IsFocusable(parent)) {
      mFocusedWidget = parent;
      return;
    }
    if (parent == mRootWidget) {
      break;
    }
    child = parent;
  }
  FocusLastWidget();
}

void FocusManager::BeforeDestroy(Widgets::Widget* widget) {
  if (mFocusedWidget != widget) {
    return;
  }

  // 1. Try previous within siblings
  // 2. Try next within siblings
  // 3. Try parent
  // 4. Goto 1
  auto child = widget;
  while (const auto parent = child->GetParent()) {
    const auto children = parent->GetChildren();
    const auto it = std::ranges::find(children, child);
    for (auto&& sibling: std::ranges::subrange(children.begin(), it)) {
      if (const auto target = FirstFocusableWidget(sibling)) {
        mFocusedWidget = target;
        return;
      }
    }
    for (auto&& sibling: std::ranges::subrange(it + 1, children.end())) {
      if (const auto target = FirstFocusableWidget(sibling)) {
        mFocusedWidget = target;
      }
      return;
    }
    if (IsFocusable(parent)) {
      mFocusedWidget = parent;
      return;
    }

    if (parent == mRootWidget) {
      break;
    }
    child = parent;
  }
  this->FocusFirstWidget();
}

void FocusManager::FocusFirstWidget() {
  mFocusedWidget = FirstFocusableWidget(mRootWidget);
}

void FocusManager::FocusLastWidget() {
  mFocusedWidget = LastFocusableWidget(mRootWidget);
}

Widgets::Widget* FocusManager::FirstFocusableWidget(Widgets::Widget* parent) {
  if (IsFocusable(parent)) {
    return parent;
  }

  for (auto&& child: parent->GetChildren()) {
    if (const auto it = FirstFocusableWidget(child)) {
      return it;
    }
  }

  return nullptr;
}

Widgets::Widget* FocusManager::LastFocusableWidget(Widgets::Widget* parent) {
  for (auto&& child: parent->GetChildren() | std::views::reverse) {
    if (const auto it = LastFocusableWidget(child)) {
      return it;
    }
  }

  if (IsFocusable(parent)) {
    return parent;
  }

  return nullptr;
}

}// namespace FredEmmott::GUI
