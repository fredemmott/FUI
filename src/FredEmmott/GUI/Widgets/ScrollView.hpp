// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ScrollBar.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ScrollView final : public Widget {
 public:
  ScrollView(std::size_t id, const StyleClasses& classes = {});
  ~ScrollView() override;

 protected:
  [[nodiscard]]
  WidgetList GetDirectChildren() const noexcept override;

  unique_ptr<ScrollBar> mHorizontalScrollBar;
  unique_ptr<Widget> mContainer;
  unique_ptr<ScrollBar> mVerticalScrollBar;
};
}// namespace FredEmmott::GUI::Widgets
