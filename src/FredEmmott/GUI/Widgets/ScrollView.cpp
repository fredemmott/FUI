// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>

namespace FredEmmott::GUI::Widgets {

ScrollView::ScrollView(std::size_t id, const StyleClasses& classes)
  : Widget(id, classes) {
  this->ChangeDirectChildren([this] {
    mContent.reset(new Widget(0));
    mHorizontalScrollBar.reset(new ScrollBar(1, Orientation::Horizontal));
    mVerticalScrollBar.reset(new ScrollBar(2, Orientation::Vertical));
  });

  using StaticTheme::ScrollBar::ScrollBarSize;

  mContent->SetBuiltInStyles({
    .mMarginBottom = ScrollBarSize,
    .mMarginRight = ScrollBarSize,
    .mOverflow = YGOverflowScroll,
    .mPosition = YGPositionTypeAbsolute,
  });
  mVerticalScrollBar->SetAdditionalBuiltInStyles({
    .mBottom = ScrollBarSize,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = 0,
    .mTop = 0,
  });
  mHorizontalScrollBar->SetAdditionalBuiltInStyles({
    .mBottom = 0.f,
    .mFlexGrow = 1,
    .mLeft = 0.f,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = ScrollBarSize,
  });
}

ScrollView::~ScrollView() = default;

WidgetList ScrollView::GetDirectChildren() const noexcept {
  return {
    mContent.get(),
    mVerticalScrollBar.get(),
    mHorizontalScrollBar.get(),
  };
}

Widget* ScrollView::GetFosterParent() const noexcept {
  return mContent.get();
}

Style ScrollView::GetBuiltInStyles() const {
  return {
    .mFlexBasis = 64,
    .mFlexGrow = 1,
  };
}
void ScrollView::PaintChildren(SkCanvas* canvas) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  canvas->save();
  canvas->clipIRect(SkIRect::MakeWH(w, h));
  mContent->Paint(canvas);
  canvas->restore();

  mHorizontalScrollBar->Paint(canvas);
  mVerticalScrollBar->Paint(canvas);
}

}// namespace FredEmmott::GUI::Widgets