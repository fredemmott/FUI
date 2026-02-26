// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/Size.hpp>
#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/yoga.hpp>

#include "FredEmmott/GUI/FocusManager.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
};

namespace FredEmmott::GUI::Immediate {

class Root final {
 public:
  Root();
  ~Root();

  void Reset();

  void BeginFrame();
  void EndFrame();
  void Paint(Renderer*, const Size&);

  [[nodiscard]]
  bool CanFit(const Size&) const;
  [[nodiscard]]
  bool CanFit(float width, float height) const;
  YGNodeRef GetLayoutNode() const;
  Size GetInitialSize() const;

  Widgets::Widget* GetWidget() const;
  FocusManager* GetFocusManager() const;

  float GetHeightForWidth(float) const;
  FrameRateRequirement GetFrameRateRequirement() const;

  Widgets::Widget* DispatchEvent(const Event*);

 private:
  struct WidgetRoot {
    unique_ptr<Widgets::Widget> mWidget;
    FocusManager mFocusManager;
  };
  std::optional<WidgetRoot> mWidgetRoot;
  unique_ptr<YGNode> mYogaRoot;
};

}// namespace FredEmmott::GUI::Immediate