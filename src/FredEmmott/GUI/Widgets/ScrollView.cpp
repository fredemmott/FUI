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

  using namespace StaticTheme::ScrollBar;

  mContent->SetBuiltInStyles({
    .mMarginBottom = ScrollBarHorizontalThumbMinHeight + 4,
    .mMarginRight = ScrollBarVerticalThumbMinWidth + 4,
    .mOverflow = YGOverflowScroll,
    .mPosition = YGPositionTypeRelative,
  });
  mVerticalScrollBar->SetBuiltInStyles({
    .mBottom = ScrollBarHorizontalThumbMinHeight + 4,
    .mPosition = YGPositionTypeRelative,
    .mRight = 0,
    .mLeft = 0,
  });
  mHorizontalScrollBar->SetBuiltInStyles({
    .mBottom = 0.f,
    .mFlexGrow = 1,
    .mLeft = 0.f,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = ScrollBarVerticalThumbMinWidth + 4,
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

}// namespace FredEmmott::GUI::Widgets