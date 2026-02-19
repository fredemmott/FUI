// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Slider final : public Widget, public IFocusable {
 public:
  enum class SnapTo {
    Steps,
    Ticks,
  };
  explicit Slider(std::size_t id = 0);

  void SetSnapTo(const SnapTo snapTo) {
    mSnapTo = snapTo;
  }
  [[nodiscard]] SnapTo GetSnapTo() const noexcept {
    return mSnapTo;
  }

  void SetValue(float value);
  [[nodiscard]] float GetValue() const;

  [[nodiscard]]
  float GetTickFrequency() const noexcept {
    return mTickFrequency;
  }
  void SetTickFrequency(float frequency);

  [[nodiscard]]
  float GetStepFrequency() const noexcept {
    return mStepFrequency;
  }
  void SetStepFrequency(float frequency);

  void SetRange(float min, float max);

  bool mChanged {false};

 protected:
  void UpdateLayout() override;
  EventHandlerResult OnMouseMove(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;

  ComputedStyleFlags OnComputedStyleChange(const Style&, StateFlags) override;
  EventHandlerResult OnKeyPress(const KeyPressEvent&) override;

  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  SnapTo mSnapTo {SnapTo::Steps};
  float mValue {0.0f};
  float mMin {0.0f};
  float mMax {100.0f};
  float mStepFrequency {1.0f};
  float mTickFrequency {};
  std::optional<float> mDraggingValue;
  Widget* mTrack {nullptr};
  Widget* mOuterThumb {nullptr};
  Widget* mInnerThumb {nullptr};

  void UpdateThumbPosition();
};

}// namespace FredEmmott::GUI::Widgets