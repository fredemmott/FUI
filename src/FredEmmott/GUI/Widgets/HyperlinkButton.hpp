// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class HyperlinkButton final : public Widget, public IInvocable {
 public:
  explicit HyperlinkButton(Window*, const StyleClasses& classes = {});
  ~HyperlinkButton() override;

  void Invoke() override;

 protected:
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
};

}// namespace FredEmmott::GUI::Widgets