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
  if (widget->IsDestructionInProgress()) {
    return false;
  }
  if (widget->IsDisabled()) {
    return false;
  }

  if (!dynamic_cast<Widgets::IFocusable const*>(widget)) {
    return false;
  }
  return !dynamic_cast<Widgets::ISelectionItem const*>(widget);
}
}// namespace

FocusManager::~FocusManager() = default;

FocusManager::FocusManager(Widgets::Widget* rootWidget)
  : mRootWidget(rootWidget) {}

std::optional<std::tuple<Widgets::Widget*, FocusKind>>
FocusManager::GetFocusedWidget() const {
  if (!mFocusedWidget) {
    return std::nullopt;
  }
  if (mFocusedWidget->IsDisabled()) {
    return std::nullopt;
  }
  return std::tuple {mFocusedWidget, mFocusKind};
}

bool FocusManager::IsWidgetFocused(const Widgets::Widget* const widget) const {
  FUI_ASSERT(widget);
  return mFocusedWidget == widget;
}

void FocusManager::GiveImplicitFocus(Widgets::Widget* widget) {
  mFocusedWidget = widget;
  mFocusKind = FocusKind::Implicit;
}

void FocusManager::GiveVisibleFocus(Widgets::Widget* w) {
  mFocusedWidget = w;
  mFocusKind = FocusKind::Visible;
}

void FocusManager::FocusNextWidget() {
  mFocusKind = FocusKind::Visible;
  if (!mFocusedWidget) {
    this->FocusFirstWidget();
    return;
  }

  const auto selectItem
    = felly::scope_exit([this] { this->FocusFirstSelectedItem(); });

  auto child = std::exchange(mFocusedWidget, nullptr);

  if (dynamic_cast<Widgets::ISelectionItem const*>(child)) {
    child = child->GetLogicalParent();
  }

  while (const auto parent = child->GetLogicalParentOrNull()) {
    const auto children = parent->GetLogicalChildren();
    const auto it = std::ranges::find(children, child);
    FUI_ASSERT(it != children.end());
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
  const auto container
    = dynamic_cast<Widgets::ISelectionContainer const*>(mFocusedWidget);
  if (!container) {
    return;
  }

  for (auto&& item: container->GetSelectionItems()) {
    const auto widget = item->GetWidget();
    if (widget->IsDisabled()) {
      continue;
    }

    if (item->IsSelected()) {
      mFocusedWidget = widget;
      return;
    }
  }

  this->FocusNextSelectionItem();
}

void FocusManager::FocusPreviousWidget() {
  mFocusKind = FocusKind::Visible;
  if (!mFocusedWidget) {
    this->FocusFirstWidget();
    return;
  }

  const auto selectItem
    = felly::scope_exit([this] { this->FocusFirstSelectedItem(); });

  auto child = std::exchange(mFocusedWidget, nullptr);
  if (const auto item = dynamic_cast<Widgets::ISelectionItem*>(child)) {
    child = item->GetSelectionContainer()->GetWidget();
  }
  while (const auto parent = child->GetLogicalParentOrNull()) {
    const auto children = parent->GetLogicalChildren();
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
  while (const auto parent = child->GetLogicalParentOrNull()) {
    const auto children = parent->GetLogicalChildren();
    const auto it = std::ranges::find(children, child);
    FUI_ASSERT(it != children.end());
    for (auto&& sibling: std::ranges::subrange(children.begin(), it)) {
      if (const auto target = FirstFocusableWidget(sibling)) {
        mFocusedWidget = target;
        return;
      }
    }
    for (auto&& sibling: std::ranges::subrange(it + 1, children.end())) {
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
          this->EnsureFocusedWidgetIsVisible();
          return true;
        case Modifier_Shift:
          this->FocusPreviousWidget();
          this->EnsureFocusedWidgetIsVisible();
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
      this->EnsureFocusedWidgetIsVisible();
      return true;
    case Key_RightArrow:
    case Key_DownArrow:
      this->FocusNextSelectionItem();
      this->EnsureFocusedWidgetIsVisible();
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

void FocusManager::FocusFirstSelectionItem(auto makeRange) {
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

  const auto item
    = dynamic_cast<Widgets::ISelectionItem const*>(mFocusedWidget);
  if (!item) {
    return;
  }

  const auto children = item->GetSelectionContainer()->GetSelectionItems();
  for (auto&& sibling: makeRange(children)) {
    const auto widget = sibling->GetWidget();
    if (widget->IsDisabled()) {
      continue;
    }

    mFocusedWidget = widget;
    mFocusKind = FocusKind::Visible;
    return;
  }
}

void FocusManager::FocusNextSelectionItem() {
  FocusFirstSelectionItem([this](const auto& children) {
    const auto it = std::ranges::find(
      children, mFocusedWidget, &Widgets::IFocusable::GetWidget);
    return std::ranges::subrange(it + 1, children.end());
  });
}

void FocusManager::FocusPreviousSelectionItem() {
  FocusFirstSelectionItem([this](const auto& children) {
    const auto it = std::ranges::find(
      children, mFocusedWidget, &Widgets::IFocusable::GetWidget);
    return std::ranges::subrange(children.begin(), it) | std::views::reverse;
  });
}

Widgets::Widget* FocusManager::FirstFocusableWidget(Widgets::Widget* parent) {
  if (parent->IsDestructionInProgress()) {
    return nullptr;
  }

  if (IsFocusable(parent)) {
    return parent;
  }

  for (auto&& child: parent->GetLogicalChildren()) {
    if (const auto it = FirstFocusableWidget(child)) {
      return it;
    }
  }

  return nullptr;
}

Widgets::Widget* FocusManager::LastFocusableWidget(Widgets::Widget* parent) {
  for (auto&& child: parent->GetLogicalChildren() | std::views::reverse) {
    if (const auto it = LastFocusableWidget(child)) {
      return it;
    }
  }

  if (IsFocusable(parent)) {
    return parent;
  }

  return nullptr;
}

void FocusManager::EnsureFocusedWidgetIsVisible() {
  if (!mFocusedWidget) {
    return;
  }
  mFocusedWidget->EnsureVisible();
}

}// namespace FredEmmott::GUI
