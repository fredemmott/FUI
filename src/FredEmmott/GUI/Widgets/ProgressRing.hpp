// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ProgressRing final : public Widget {
 public:
  enum class Kind {
    Indeterminate,
    Determinate,
  };
  ProgressRing(Window*, Kind);
  ~ProgressRing() override;

  void SetRange(float minimum, float maximum);
  void SetValue(float value);
  // Only affects Indeterminate
  void SetIsActive(bool value);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;
  FrameRateRequirement GetFrameRateRequirement() const noexcept override;

 private:
  Kind mKind {Kind::Indeterminate};
  bool mIsActive {true};
  float mMinimum {0.0f};
  float mMaximum {100.0f};
  float mValue {0.0f};
};
}// namespace FredEmmott::GUI::Widgets