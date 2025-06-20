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
    mVerticalScrollBar.get(),
    mHorizontalScrollBar.get(),
    mContent.get(),
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

void ScrollView::BeforeFrame() {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);
  const auto contentNode = mContent->GetLayoutNode();
  const auto cw = YGNodeLayoutGetWidth(contentNode);
  const auto ch = YGNodeLayoutGetHeight(contentNode);

  const auto showHScroll
    = (w > 0 && cw > 0) && (w - cw < std::numeric_limits<float>::epsilon());
  const auto showVScroll
    = (h > 0 && ch > 0) && (h - ch < std::numeric_limits<float>::epsilon());

  if (showHScroll) {
    mHorizontalScrollBar->SetThumbSize(w);
    mHorizontalScrollBar->SetMaximum(cw - w);
  } else {
    mHorizontalScrollBar->SetValue(0);
  }

  if (showVScroll) {
    mVerticalScrollBar->SetThumbSize(h);
    mVerticalScrollBar->SetMaximum(ch - h);
  } else {
    mVerticalScrollBar->SetValue(0);
  }

  Widget::BeforeFrame();
}

void ScrollView::PaintChildren(SkCanvas* canvas) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  canvas->save();
  canvas->clipRect(SkRect::MakeWH(w, h));
  canvas->translate(
    -mHorizontalScrollBar->GetValue(), -mVerticalScrollBar->GetValue());
  mContent->Paint(canvas);
  canvas->restore();

  mHorizontalScrollBar->Paint(canvas);
  mVerticalScrollBar->Paint(canvas);
}

}// namespace FredEmmott::GUI::Widgets