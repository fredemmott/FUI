// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

/// Any widget that can receive focus
class IFocusable {
 public:
  IFocusable() = delete;
  explicit constexpr IFocusable(Widget* widget) : mWidget(widget) {
    FUI_ASSERT(widget);
  }

  virtual ~IFocusable() = default;

  constexpr Widget* GetWidget() const noexcept {
    return mWidget;
  }

 private:
  Widget* mWidget {};
};

/// Widgets with a single unambiguous action, e.g. buttons
class IInvocable : public IFocusable {
 public:
  using IFocusable::IFocusable;
  virtual void Invoke() = 0;
};

/// Widgets with multiple states that can be rotated through, e.g. checkboxes
class IToggleable : public IFocusable {
 public:
  using IFocusable::IFocusable;

  virtual void Toggle() = 0;
};

class ISelectionItem;

class ISelectionContainer : public IFocusable {
 public:
  using IFocusable::IFocusable;

  [[nodiscard]]
  virtual std::vector<ISelectionItem*> GetSelectionItems() const noexcept = 0;

 protected:
  template <std::derived_from<Widget> TItemWidget>
    requires std::derived_from<TItemWidget, ISelectionItem>
  static ISelectionItem* CastToSelectionItem(Widget* item) {
    if constexpr (Config::Debug) {
      const auto refined = dynamic_cast<TItemWidget*>(item);
      FUI_ASSERT(refined, "Got unexpected item type in ISelectionContainer");
      return refined;
    } else {
      return static_cast<TItemWidget*>(item);
    }
  }
};

/// A single item that can be selected from a list, e.g. a single radio button
class ISelectionItem : public IFocusable {
 public:
  using IFocusable::IFocusable;

  [[nodiscard]]
  virtual bool IsSelected() const noexcept = 0;
  virtual void Select() = 0;
  [[nodiscard]]
  virtual bool ConsumeWasSelected() noexcept = 0;

  [[nodiscard]]
  virtual ISelectionContainer* GetSelectionContainer() const noexcept = 0;

 protected:
  template <std::derived_from<Widget> TContainerWidget>
    requires std::derived_from<TContainerWidget, ISelectionContainer>
  static ISelectionContainer* CastToSelectionContainer(Widget* container) {
    if constexpr (Config::Debug) {
      const auto refined = dynamic_cast<TContainerWidget*>(container);
      FUI_ASSERT(refined, "Got unexpected container type in ISelectionItem");
      return refined;
    } else {
      return static_cast<TContainerWidget*>(container);
    }
  }
};

}// namespace FredEmmott::GUI::Widgets
