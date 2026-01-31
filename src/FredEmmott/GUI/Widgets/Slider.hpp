// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Slider final : public Widget {
 public:
  explicit Slider(std::size_t id = 0);

  void SetValue(float value);
  [[nodiscard]] float GetValue() const;

  [[nodiscard]]
  float GetTickFrequency() const {
    return mTickFrequency;
  }
  void SetTickFrequency(float frequency);

  void SetRange(float min, float max);

  bool mChanged {false};

 protected:
  void UpdateLayout() override;
  EventHandlerResult OnMouseMove(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;

  ComputedStyleFlags OnComputedStyleChange(const Style&, StateFlags) override;

  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  void UpdateThumbPosition();

  float mValue {0.0f};
  float mMin {0.0f};
  float mMax {100.0f};
  float mTickFrequency {};
  bool mIsDragging {false};
  Widget* mTrack {nullptr};
  Widget* mOuterThumb {nullptr};
  Widget* mInnerThumb {nullptr};
};

}// namespace FredEmmott::GUI::Widgets