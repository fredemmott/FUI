// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>
#include <skia/core/SkSize.h>

#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/yoga.hpp>
#include <optional>

namespace FredEmmott::GUI::Widgets {
class Widget;
};

namespace FredEmmott::GUI::Immediate {

class Root final {
 public:
  Root();
  ~Root();
  void BeginFrame();
  void EndFrame();
  void Paint(SkCanvas*, SkSize);

  [[nodiscard]]
  bool CanFit(const SkSize&) const;
  [[nodiscard]]
  bool CanFit(float width, float height) const;
  SkSize GetInitialSize() const;

  float GetHeightForWidth(float) const;
  FrameRateRequirement GetFrameRateRequirement() const;

  void DispatchEvent(const Event*);

 private:
  unique_ptr<Widgets::Widget> mWidget;
  unique_ptr<YGNode> mYogaRoot;
  enum class Cursor {
    Default,
  };
};

}// namespace FredEmmott::GUI::Immediate