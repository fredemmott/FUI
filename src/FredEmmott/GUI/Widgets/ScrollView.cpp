// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"

namespace FredEmmott::GUI::Widgets {

ScrollView::ScrollView(std::size_t id, const StyleClasses& classes)
  : Widget(id, classes) {
  this->ChangeDirectChildren([this] {
    mHorizontalScrollBar.reset(new ScrollBar(0, Orientation::Horizontal));
    mVerticalScrollBar.reset(new ScrollBar(1, Orientation::Vertical));
    mContent.reset(new Widget(2));
  });
  YGNodeRemoveAllChildren(this->GetLayoutNode());
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &ScrollView::Measure);
  {
    const auto child = mContent->GetLayoutNode();
    mContentYoga.reset(YGNodeNew());
    YGNodeStyleSetOverflow(mContentYoga.get(), YGOverflowScroll);
    YGNodeSetChildren(mContentYoga.get(), &child, 1);

    YGNodeSetContext(
      mContentYoga.get(),
      new widget_detail::YogaContext {
        widget_detail::DetachedYogaTree {
          .mRealParent = this->GetLayoutNode(),
        },
      });
  }
  {
    mScrollBarsYoga.reset(YGNodeNew());
    const std::array scrollbars {
      mHorizontalScrollBar->GetLayoutNode(),
      mVerticalScrollBar->GetLayoutNode(),
    };
    YGNodeSetChildren(
      mScrollBarsYoga.get(), scrollbars.data(), scrollbars.size());
  }

  using StaticTheme::ScrollBar::ScrollBarSize;

  mVerticalScrollBar->SetAdditionalBuiltInStyles({
    .mBottom = ScrollBarSize,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = 4,
    .mTop = 0,
  });
  mHorizontalScrollBar->SetAdditionalBuiltInStyles({
    .mBottom = 4,
    .mFlexGrow = 1,
    .mLeft = 0.f,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = ScrollBarSize,
  });
}

ScrollView::~ScrollView() {
  const auto ctx = static_cast<widget_detail::YogaContext*>(
    YGNodeGetContext(mContentYoga.get()));
  YGNodeSetContext(mContentYoga.get(), nullptr);
  delete ctx;
}

ScrollView::ScrollBarVisibility ScrollView::GetHorizontalScrollBarVisibility()
  const noexcept {
  return mHorizontalScrollBarVisibility;
}

ScrollView::ScrollBarVisibility ScrollView::GetVerticalScrollBarVisibility()
  const noexcept {
  return mVerticalScrollBarVisibility;
}

ScrollView& ScrollView::SetHorizontalScrollBarVisibility(
  const ScrollBarVisibility value) noexcept {
  mHorizontalScrollBarVisibility = value;
  return *this;
}

ScrollView& ScrollView::SetVerticalScrollBarVisibility(
  const ScrollBarVisibility value) noexcept {
  mVerticalScrollBarVisibility = value;
  return *this;
}

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

void ScrollView::UpdateLayout() {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);
  YGNodeCalculateLayout(mContentYoga.get(), w, YGUndefined, YGDirectionLTR);
  YGNodeCalculateLayout(mScrollBarsYoga.get(), w, h, YGDirectionLTR);

  const auto cw = YGNodeLayoutGetWidth(mContentYoga.get());
  const auto ch = YGNodeLayoutGetHeight(mContentYoga.get());

  const bool showHScroll
    = IsScrollBarVisible(mHorizontalScrollBarVisibility, cw, w);
  const bool showVScroll
    = IsScrollBarVisible(mVerticalScrollBarVisibility, ch, h);
  mHorizontalScrollBar->AddExplicitStyles(
    Style {
      .mDisplay = showHScroll ? YGDisplayFlex : YGDisplayNone,
    });
  mVerticalScrollBar->AddExplicitStyles(
    Style {
      .mDisplay = showVScroll ? YGDisplayFlex : YGDisplayNone,
    });

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

  Widget::UpdateLayout();
}

void ScrollView::PaintChildren(SkCanvas* canvas) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);
  YGNodeCalculateLayout(mContentYoga.get(), w, h, YGDirectionLTR);
  YGNodeCalculateLayout(mScrollBarsYoga.get(), w, h, YGDirectionLTR);

  const SkPoint offset {
    mHorizontalScrollBar.get()->GetValue(),
    mVerticalScrollBar.get()->GetValue(),
  };
  mContent->ScrollTo(offset);
  std::get<widget_detail::DetachedYogaTree>(
    *static_cast<widget_detail::YogaContext*>(
      YGNodeGetContext(mContentYoga.get())))
    .mOffset
    = offset;

  canvas->save();
  canvas->clipRect(SkRect::MakeWH(w, h));
  mContent->Paint(canvas);
  canvas->restore();

  mHorizontalScrollBar->Paint(canvas);
  mVerticalScrollBar->Paint(canvas);
}

bool ScrollView::IsScrollBarVisible(
  const ScrollBarVisibility visibility,
  const float content,
  const float container) noexcept {
  if (visibility == ScrollBarVisibility::Visible) {
    return true;
  }
  if (visibility == ScrollBarVisibility::Hidden) {
    return false;
  }

  constexpr auto eps = std::numeric_limits<float>::epsilon();
  if (content <= eps || container <= eps) {
    return false;
  }

  return (content - container) > eps;
}
YGSize ScrollView::Measure(
  YGNodeConstRef node,
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  auto& self = *static_cast<ScrollView*>(FromYogaNode(node));

  auto root = self.mContentYoga.get();
  if (height == 0) {
    height = YGUndefined;
  }
  YGNodeCalculateLayout(root, width, height, YGDirectionLTR);
  width = YGNodeLayoutGetWidth(root);
  height = YGNodeLayoutGetHeight(root);
  return {width, height};
}

}// namespace FredEmmott::GUI::Widgets