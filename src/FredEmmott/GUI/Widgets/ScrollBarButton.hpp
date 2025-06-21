// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <functional>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ScrollBarButton final : public Widget {
 public:
  /** Construct a ScrollBarButton
   *
   * - `pressCallback` will be invoked when the mouse button is pressed
   * - `tickCallback` will be invoked when the mouse button is pressed, and
   *   at recurring intervals after a delay, if the mouse button is still held
   *   down.
   */
  ScrollBarButton(
    std::function<void(const SkPoint&)> pressCallback,
    std::function<void()> tickCallback,
    std::size_t id);
  ~ScrollBarButton() override;

  void SetText(std::string_view);

  FrameRateRequirement GetFrameRateRequirement() const noexcept override;

  void Tick() override;

 protected:
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;

 private:
  std::optional<std::chrono::steady_clock::time_point> mNextTick;
  std::function<void(const SkPoint&)> mPressCallback;
  std::function<void()> mTickCallback;
};

}// namespace FredEmmott::GUI::Widgets