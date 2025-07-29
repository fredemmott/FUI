// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FocusManager.hpp"

#include <stack>

#include "Widgets/Focusable.hpp"
#include "assert.hpp"
#include "events/KeyEvent.hpp"

namespace FredEmmott::GUI {

namespace {
bool IsFocusable(Widgets::Widget const* widget) {
  if (widget->IsDisabled()) {
    return false;
  }
  if (!dynamic_cast<Widgets::IFocusable const*>(widget)) {
    return false;
  }
  return !dynamic_cast<Widgets::ISelectionItem const*>(widget);
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

  const auto selectItem
    = wil::scope_exit([this] { this->FocusFirstSelectedItem(); });

  auto child = std::exchange(mFocusedWidget, nullptr);
  if (dynamic_cast<Widgets::ISelectionItem const*>(child)) {
    child = child->GetParent();
  }
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

void FocusManager::FocusFirstSelectedItem() {
  if (!dynamic_cast<Widgets::ISelectionContainer const*>(mFocusedWidget)) {
    return;
  }

  for (auto&& child: mFocusedWidget->GetChildren()) {
    const auto it = dynamic_cast<Widgets::ISelectionItem const*>(child);
    if (it && it->IsSelected()) {
      mFocusedWidget = child;
      return;
    }
  }

  for (auto&& child: mFocusedWidget->GetChildren()) {
    if (child->IsDisabled()) {
      continue;
    }
    const auto it = dynamic_cast<Widgets::ISelectionItem const*>(child);
    if (it) {
      mFocusedWidget = child;
      return;
    }
  }
}

void FocusManager::FocusPreviousWidget() {
  mFocusKind = FocusKind::Keyboard;
  if (!mFocusedWidget) {
    this->FocusFirstWidget();
    return;
  }

  const auto selectItem
    = wil::scope_exit([this] { this->FocusFirstSelectedItem(); });

  auto child = std::exchange(mFocusedWidget, nullptr);
  if (dynamic_cast<Widgets::ISelectionItem*>(child)) {
    child = child->GetParent();
  }
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
bool FocusManager::OnKeyPress(const KeyPressEvent& e) {
  using enum KeyCode;
  using enum KeyModifier;
  switch (e.mKeyCode) {
    case Key_Tab:
      switch (e.mModifiers) {
        case Modifier_None:
          this->FocusNextWidget();
          return true;
        case Modifier_Shift:
          this->FocusPreviousWidget();
          return true;
        default:
          break;
      }
      break;
    case Key_Return:
    case Key_Space: {
      if (mFocusedWidget && !mFocusedWidget->IsDisabled()) {
        if (
          const auto it = dynamic_cast<Widgets::IInvocable*>(mFocusedWidget)) {
          it->Invoke();
          return true;
        }
        if (
          const auto it = dynamic_cast<Widgets::IToggleable*>(mFocusedWidget)) {
          it->Toggle();
          return true;
        }
        if (
          const auto it
          = dynamic_cast<Widgets::ISelectionItem*>(mFocusedWidget)) {
          it->Select();
          return true;
        }
      }
      break;
    }
    case Key_LeftArrow:
    case Key_UpArrow:
      this->FocusPreviousSelectionItem();
      return true;
    case Key_RightArrow:
    case Key_DownArrow:
      this->FocusNextSelectionItem();
      return true;
    default:
      break;
  }
  return false;
}

void FocusManager::FocusFirstWidget() {
  mFocusedWidget = FirstFocusableWidget(mRootWidget);
  FocusFirstSelectedItem();
}

void FocusManager::FocusLastWidget() {
  mFocusedWidget = LastFocusableWidget(mRootWidget);
}

void FocusManager::FocusNextSelectionItem() {
  if (!mFocusedWidget) {
    const auto w = FirstFocusableWidget(mRootWidget);
    if (!dynamic_cast<Widgets::ISelectionContainer const*>(w)) {
      return;
    }
    mFocusedWidget = w;
    FocusFirstSelectedItem();
    if (!dynamic_cast<Widgets::ISelectionItem const*>(mFocusedWidget)) {
      mFocusedWidget = nullptr;
      return;
    }
  }

  if (!dynamic_cast<Widgets::ISelectionItem const*>(mFocusedWidget)) {
    return;
  }

  const auto children = mFocusedWidget->GetParent()->GetChildren();
  const auto it = std::ranges::find(children, mFocusedWidget);
  for (auto&& sibling: std::ranges::subrange(it + 1, children.end())) {
    if (sibling->IsDisabled()) {
      continue;
    }
    if (dynamic_cast<Widgets::ISelectionItem const*>(sibling)) {
      mFocusedWidget = sibling;
      mFocusKind = FocusKind::Keyboard;
      return;
    }
  }
}

void FocusManager::FocusPreviousSelectionItem() {
  if (!mFocusedWidget) {
    const auto w = FirstFocusableWidget(mRootWidget);
    if (!dynamic_cast<Widgets::ISelectionContainer const*>(w)) {
      return;
    }
    mFocusedWidget = w;
    FocusFirstSelectedItem();
    if (!dynamic_cast<Widgets::ISelectionItem const*>(mFocusedWidget)) {
      mFocusedWidget = nullptr;
      return;
    }
  }

  if (!dynamic_cast<Widgets::ISelectionItem const*>(mFocusedWidget)) {
    return;
  }

  const auto children = mFocusedWidget->GetParent()->GetChildren();
  const auto it = std::ranges::find(children, mFocusedWidget);
  for (auto&& sibling:
       std::ranges::subrange(children.begin(), it) | std::views::reverse) {
    if (sibling->IsDisabled()) {
      continue;
    }
    if (dynamic_cast<Widgets::ISelectionItem const*>(sibling)) {
      mFocusedWidget = sibling;
      mFocusKind = FocusKind::Keyboard;
      return;
    }
  }
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
