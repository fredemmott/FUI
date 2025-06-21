// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ScrollBar.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ScrollView final : public Widget {
 public:
  enum class ScrollBarVisibility {
    Auto,
    Visible,
    Hidden,
  };

  explicit ScrollView(std::size_t id, const StyleClasses& classes = {});
  ~ScrollView() override;

  [[nodiscard]]
  ScrollBarVisibility GetHorizontalScrollBarVisibility() const noexcept;
  ScrollView& SetHorizontalScrollBarVisibility(ScrollBarVisibility) noexcept;

  [[nodiscard]]
  ScrollBarVisibility GetVerticalScrollBarVisibility() const noexcept;
  ScrollView& SetVerticalScrollBarVisibility(ScrollBarVisibility) noexcept;

 protected:
  [[nodiscard]]
  WidgetList GetDirectChildren() const noexcept override;
  Widget* GetFosterParent() const noexcept override;
  void UpdateLayout() override;
  void PaintChildren(SkCanvas* canvas) const override;
  EventHandlerResult OnMouseVerticalWheel(const MouseEvent&) override;

  [[nodiscard]]
  static bool IsScrollBarVisible(
    ScrollBarVisibility visibility,
    float content,
    float container) noexcept;

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);

  unique_ptr<Widget> mContent;
  unique_ptr<YGNode> mContentYoga;

  unique_ptr<YGNode> mScrollBarsYoga;
  unique_ptr<ScrollBar> mHorizontalScrollBar;
  unique_ptr<ScrollBar> mVerticalScrollBar;

  ScrollBarVisibility mHorizontalScrollBarVisibility {
    ScrollBarVisibility::Hidden};
  ScrollBarVisibility mVerticalScrollBarVisibility {
    ScrollBarVisibility::Hidden};
};
}// namespace FredEmmott::GUI::Widgets
