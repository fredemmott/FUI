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
    return &mWindow;
  }

 protected:
  WidgetList GetDirectChildren() const noexcept override;
  Style GetBuiltInStyles() const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  unique_ptr<Widget> mRootWidget;
  Window mWindow {
    GetModuleHandleW(nullptr),
    SW_SHOWNA,
    {
      .mWindowStyle = WS_POPUP | WS_BORDER,
      .mWindowExStyle = WS_EX_NOREDIRECTIONBITMAP | WS_EX_NOACTIVATE,
      .mSystemBackdrop = DWMSBT_TRANSIENTWINDOW,
    }};
};
}// namespace FredEmmott::GUI::Widgets
