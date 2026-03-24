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
  Root() = delete;
  Root(Widgets::Widget* root, Widgets::Widget* immediateRoot);
  ~Root();

  void Reset();

  void BeginFrame();
  void EndFrame();
  void Paint(Renderer*, const Size&);

  [[nodiscard]]
  bool CanFit(const Size&) const;
  [[nodiscard]]
  bool CanFit(float width, float height) const;
  YGNode* GetLayoutNode() const;
  Size GetInitialSize() const;

  FocusManager* GetFocusManager() const;

  Widgets::Widget* GetImplementationRoot() const {
    return mActualRoot;
  }

  Widgets::Widget* GetImmediateRoot() const {
    return mImmediateRoot;
  }

  float GetHeightForWidth(float) const;
  FrameRateRequirement GetFrameRateRequirement() const;

  Widgets::Widget* DispatchEvent(const Event&);

 private:
  Widgets::Widget* mActualRoot {};
  Widgets::Widget* mImmediateRoot {};
  FocusManager mFocusManager;
  unique_yoga_node_ptr mYogaRoot;
};

}// namespace FredEmmott::GUI::Immediate