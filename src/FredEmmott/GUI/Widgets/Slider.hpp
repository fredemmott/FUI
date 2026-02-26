// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "FredEmmott/GUI/Orientation.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Slider final : public Widget, public IFocusable {
 public:
  enum class SnapTo {
    Steps,
    Ticks,
  };
  Slider() = delete;
  explicit Slider(id_type id, Orientation);

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
    return std::tuple(mMin, mMax);
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
  float GetDraggingTrackOffset() const {
    return mDraggingTrackOffset.value();
  }
  [[nodiscard]]
  float GetTrackLength() const;

  [[nodiscard]]
  bool ConsumeWasThumbHovered() noexcept {
    const auto outerHovered
      = std::exchange(mOuterThumb->mWasStationaryHovered, std::nullopt)
          .has_value();
    const auto innerHovered
      = std::exchange(mInnerThumb->mWasStationaryHovered, std::nullopt)
          .has_value();
    return outerHovered || innerHovered;
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
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;

  ComputedStyleFlags OnComputedStyleChange(const Style&, StateFlags) override;
  EventHandlerResult OnKeyPress(const KeyPressEvent&) override;

  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  Orientation mOrientation {Orientation::Horizontal};
  SnapTo mSnapTo {SnapTo::Steps};
  float mValue {0.0f};
  float mMin {0.0f};
  float mMax {100.0f};
  float mStepFrequency {1.0f};
  float mTickFrequency {};
  std::optional<float> mDraggingValue;
  std::optional<float> mDraggingTrackOffset;
  Widget* mTrack {nullptr};
  Widget* mOuterThumb {nullptr};
  Widget* mInnerThumb {nullptr};
  Widget* mBeforeThumb {nullptr};
  Widget* mAfterThumb {nullptr};

  bool mWasChanged {false};
  bool mDidReceiveKeyboardInput {false};

  void UpdateThumbPosition();
  [[nodiscard]]
  float GetSnappedValue(float) const noexcept;
};

}// namespace FredEmmott::GUI::Widgets