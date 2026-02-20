// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Orientation.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Label;
class ScrollBarButton;
class ScrollBarThumb;

class ScrollBar final : public Widget {
 public:
  enum class ChangeReason {
    Programmatic,
    Continuous,// e.g. drag
    Discrete,// e.g. Click
  };
  using ValueChangedCallback = std::function<void(float, ChangeReason)>;

  ScrollBar(std::size_t id, Orientation);
  ScrollBar(std::size_t id, const ImmutableStyle&, Orientation);

  static ImmutableStyle MakeImmutableStyle(Orientation, const Style& mixin);

  ~ScrollBar() override;

  void SetMinimum(float value);
  [[nodiscard]] float GetMinimum() const;
  void SetMaximum(float maximum);
  [[nodiscard]] float GetMaximum() const;
  void SetValue(float value);
  [[nodiscard]] float GetValue() const;
  void SetThumbSize(float value);
  [[nodiscard]] float GetThumbSize() const;
  void OnValueChanged(ValueChangedCallback);

 protected:
  [[nodiscard]]
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  Orientation mOrientation;

  float mMinimum {0.0f};
  float mMaximum {1.0f};
  float mValue {0.0f};
  float mThumbSize {0.0f};

  ScrollBarButton* mSmallDecrement {};// Arrow
  Widget* mTrack {};
  ScrollBarButton* mSmallIncrement {};// Arrow

  // These are within the track
  ScrollBarButton* mLargeDecrement {nullptr};// Space above thumb
  ScrollBarThumb* mThumb {nullptr};
  ScrollBarButton* mLargeIncrement {nullptr};// Space below thumb

  ValueChangedCallback mValueChangedCallback;

  std::optional<float> mLargeDecrementMin;
  std::optional<float> mLargeIncrementMax;

  enum class ButtonTickKind {
    SmallDecrement,
    LargeDecrement,
    LargeIncrement,
    SmallIncrement,
  };

  void ScrollBarButtonTick(ButtonTickKind);
  void ScrollBarButtonDown(ButtonTickKind, const Point&);

  void SetValue(float value, ChangeReason);

  void UpdateChildSizes();

  void OnThumbDrag(Point* delta);
};

}// namespace FredEmmott::GUI::Widgets