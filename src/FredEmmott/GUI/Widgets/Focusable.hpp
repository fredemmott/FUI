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
class IInvocable : public IFocusable{
 public:
  virtual void Invoke() = 0;
};

/// Widgets with multiple states that can be rotated through, e.g. checkboxes
class IToggleable: public IFocusable {
public:
  virtual void Toggle() = 0;
};

}// namespace FredEmmott::GUI::Widgets
