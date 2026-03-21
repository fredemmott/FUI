// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

/// Base class for the back button and the pane toggle button
class NavigationViewButton : public Widget, public IInvocable {
 public:
  explicit NavigationViewButton(id_type id, std::string_view glyph);
  ~NavigationViewButton() override;

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    final;
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) final;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) final;
  EventHandlerResult OnClick(const MouseEvent&) final;

  FrameRateRequirement GetFrameRateRequirement() const noexcept final;

 private:
  std::string mGlyph;
  Font mFont {};
  Point mTextOffsetFromCenter {};

  float mFromScale {1.0f};
  float mToScale {1.0f};
  std::chrono::steady_clock::time_point mAnimationStart {};
};

}// namespace FredEmmott::GUI::Widgets