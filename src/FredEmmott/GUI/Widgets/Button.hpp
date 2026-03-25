// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button : public Widget, public IInvocable {
 public:
  explicit Button(Window*);
  Button(
    Window*,
    StyleClass primaryStyleClass,
    const ImmutableStyle&,
    const StyleClasses& = {});
  ~Button() override;

  static ImmutableStyle MakeImmutableStyle(const Style& mixin);

  void Invoke() override;

 protected:
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
};

}// namespace FredEmmott::GUI::Widgets
