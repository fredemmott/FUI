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
  WidgetStyles GetDefaultStyles() const override;

 private:
  unique_ptr<Widget> mRootWidget;
  Window mWindow {GetModuleHandleW(nullptr), SW_SHOW};
};
}// namespace FredEmmott::GUI::Widgets
