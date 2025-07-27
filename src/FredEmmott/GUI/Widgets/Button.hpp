// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button : public Widget, public IInvocable {
 public:
  explicit Button(std::size_t id);
  explicit Button(std::size_t id, const ImmutableStyle&, const StyleClasses&);

  static ImmutableStyle MakeImmutableStyle(const Style& mixin);

  bool mClicked {false};

  void Invoke() override;

 protected:
  EventHandlerResult OnClick(const MouseEvent& e) override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
};

}// namespace FredEmmott::GUI::Widgets
