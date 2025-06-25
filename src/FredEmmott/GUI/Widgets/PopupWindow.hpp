// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Window.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class PopupWindow final : public Widget {
 public:
  PopupWindow(std::size_t id);

  Window* GetWindow() {
    return mWindow.get();
  }

 protected:
  WidgetList GetDirectChildren() const noexcept override;
  Style GetBuiltInStyles() const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  unique_ptr<Widget> mRootWidget;
  unique_ptr<Window> mWindow;
};
}// namespace FredEmmott::GUI::Widgets
