// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

/// Any widget that can receive focus
class IFocusable {
 public:
  virtual ~IFocusable() = default;
};

/// Widgets with a single unambiguous action, e.g. buttons
class IInvocable : public IFocusable {
 public:
  virtual void Invoke() = 0;
};

/// Widgets with multiple states that can be rotated through, e.g. checkboxes
class IToggleable : public IFocusable {
 public:
  virtual void Toggle() = 0;
};

class ISelectionItem;

class ISelectionContainer : public IFocusable {};

/// A single item that can be selected from a list, e.g. a single radio button
class ISelectionItem : public IFocusable {
 public:
  [[nodiscard]]
  virtual bool IsSelected() const noexcept
    = 0;
  virtual void Select() = 0;
};

}// namespace FredEmmott::GUI::Widgets
