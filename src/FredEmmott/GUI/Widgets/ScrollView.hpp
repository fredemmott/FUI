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

  explicit ScrollView(id_type id, const StyleClasses& classes = {});
  ~ScrollView() override;

  [[nodiscard]]
  ScrollBarVisibility GetHorizontalScrollBarVisibility() const noexcept;
  ScrollView& SetHorizontalScrollBarVisibility(ScrollBarVisibility) noexcept;

  [[nodiscard]]
  ScrollBarVisibility GetVerticalScrollBarVisibility() const noexcept;
  ScrollView& SetVerticalScrollBarVisibility(ScrollBarVisibility) noexcept;

 protected:
  Widget* GetFosterParent() const noexcept override;
  void PaintChildren(Renderer* renderer) const override;
  EventHandlerResult OnMouseVerticalWheel(const MouseEvent&) override;

 private:
  Widget* mContentOuter {};
  Widget* mContentInner {};
  unique_ptr<YGNode> mContentYoga {};

  ScrollBar* mHorizontalScrollBar {};
  ScrollBar* mVerticalScrollBar {};

  ScrollBarVisibility mHorizontalScrollBarVisibility {
    ScrollBarVisibility::Hidden};
  ScrollBarVisibility mVerticalScrollBarVisibility {
    ScrollBarVisibility::Hidden};

  bool mDirtyInner = true;
  float mContentInnerMinWidth {};

  void OnHorizontalScroll(float value, ScrollBar::ChangeReason reason);
  void OnVerticalScroll(float value, ScrollBar::ChangeReason reason);

  void UpdateScrollBars(const Size& containerSize) const;
  void UpdateLayout() const;

  [[nodiscard]]
  static bool IsScrollBarVisible(
    ScrollBarVisibility visibility,
    float content,
    float container) noexcept;

  static void OnInnerContentDirty(YGNodeConstRef);
  static YGSize MeasureOuterContent(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};
}// namespace FredEmmott::GUI::Widgets
