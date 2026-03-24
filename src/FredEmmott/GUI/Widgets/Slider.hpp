// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "FredEmmott/GUI/Orientation.hpp"
#include "FredEmmott/GUI/StaticTheme/Slider.hpp"
#include "FredEmmott/GUI/detail/Widget/transitions.hpp"
#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Slider final : public Widget, public IFocusable {
 public:
  enum class SnapTo {
    Steps,
    Ticks,
  };
  Slider(Window*, Orientation);
  ~Slider() override;

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
  [[nodiscard]]
  auto GetRange() const noexcept {
    return std::tuple(mMinimum, mMaximum);
  }

  [[nodiscard]]
  bool IsDragging() const noexcept {
    return mDraggingValue.has_value();
  }

  [[nodiscard]]
  float GetSnappedDraggingValue() const;
  [[nodiscard]]
  Point GetTrackOriginOffset() const;
  [[nodiscard]]
  float GetThumbCenterOffsetWithinTrack() const;
  [[nodiscard]]
  float GetTrackLength() const;

  // Used to trigger tooltips
  [[nodiscard]]
  bool ConsumeWasThumbStationaryHovered() noexcept {
    return std::exchange(mWasThumbStationaryHovered, false);
  }
  [[nodiscard]]
  bool ConsumeWasChanged() noexcept {
    return std::exchange(mWasChanged, false);
  }

  [[nodiscard]]
  bool ConsumeDidReceiveKeyboardInput() noexcept {
    return std::exchange(mDidReceiveKeyboardInput, false);
  }

 protected:
  EventHandlerResult OnMouseMove(const MouseEvent&) override;
  EventHandlerResult OnMouseHover(const MouseEvent&) override;
  void OnMouseLeave(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;

  ComputedStyleFlags OnComputedStyleChange(const Style&, StateFlags) override;
  EventHandlerResult OnKeyPress(const KeyPressEvent&) override;
  FrameRateRequirement GetFrameRateRequirement() const noexcept override;

  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  struct Layout {
    float mTrackStart {};
    float mThumbMin {};
    float mThumbMax {};
    float mTrackEnd {};

    float mThumbPoint {};
  };

  Orientation mOrientation {Orientation::Horizontal};

  SnapTo mSnapTo {SnapTo::Steps};
  float mValue {0.0f};
  float mMinimum {0.0f};
  float mMaximum {100.0f};
  float mStepFrequency {1.0f};
  float mTickFrequency {};
  std::optional<float> mDraggingValue;

  bool mWasChanged {};
  bool mDidReceiveKeyboardInput {};

  uint8_t mThumbState {};
  widget_detail::TransitionState<float> mInnerThumbScale {};
  bool mWasThumbStationaryHovered {};

  [[nodiscard]]
  float GetSnappedValue(float) const noexcept;
  [[nodiscard]]
  Layout GetLayout(float length) const noexcept;

  void SetThumbState(uint8_t);
};

}// namespace FredEmmott::GUI::Widgets