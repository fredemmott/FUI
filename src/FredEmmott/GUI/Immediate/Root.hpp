// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/Size.hpp>
#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/yoga.hpp>

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
  void Paint(Renderer*, const Size&);

  [[nodiscard]]
  bool CanFit(const Size&) const;
  [[nodiscard]]
  bool CanFit(float width, float height) const;
  Size GetInitialSize() const;

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