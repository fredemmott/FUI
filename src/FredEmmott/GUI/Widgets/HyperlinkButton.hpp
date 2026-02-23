// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class HyperlinkButton final : public Widget, public IInvocable {
 public:
  HyperlinkButton(std::size_t id, const StyleClasses& classes = {});
  ~HyperlinkButton() override;

  void Invoke() override;

  [[nodiscard]]
  bool ConsumeWasActivated() noexcept {
    return std::exchange(mWasActivated, false);
  }

 protected:
  [[nodiscard]] EventHandlerResult OnClick(const MouseEvent&) override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  bool mWasActivated {false};
};

}// namespace FredEmmott::GUI::Widgets