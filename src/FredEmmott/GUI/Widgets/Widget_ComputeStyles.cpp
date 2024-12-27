// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <core/SkRRect.h>

#include "FredEmmott/GUI/detail/Widget/transitions.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

namespace {
template <class T>
struct yoga_default_value_t;
template <>
struct yoga_default_value_t<SkScalar> : constant_t<YGUndefined> {};
template <>
struct yoga_default_value_t<YGDisplay> : constant_t<YGDisplayFlex> {};

}// namespace

void Widget::ComputeStyles(const WidgetStyles& inherited) {
  WidgetStyles merged = this->GetDefaultStyles();
  merged += inherited;
  merged += mExplicitStyles;

  constexpr auto HoverFlags
    = StateFlags::Hovered | StateFlags::HoveredInherited;
  const bool isHovered = (mStateFlags & HoverFlags) != StateFlags::Default;
  constexpr auto ActiveFlags = StateFlags::Active | StateFlags::ActiveInherited;
  const bool isActive = (mStateFlags & ActiveFlags) != StateFlags::Default;

  auto style = merged.mBase;
  if (isHovered) {
    style += merged.mHover;
  }
  if (isActive) {
    style += merged.mActive;
  }

  const auto flattenEdge = [&style]<class T>(T allEdges, T thisEdge) {
    auto& thisEdgeOpt = style.*thisEdge;
    const auto& allEdgesOpt = style.*allEdges;

    thisEdgeOpt = allEdgesOpt + thisEdgeOpt;
  };
  const auto flattenEdges
    = [&flattenEdge]<class T, std::same_as<T>... TRest>(T all, TRest... rest) {
        (flattenEdge(all, rest), ...);
      };
#define FLATTEN_EDGES(X) \
  flattenEdges( \
    &Style::m##X, \
    &Style::m##X##Left, \
    &Style::m##X##Top, \
    &Style::m##X##Right, \
    &Style::m##X##Bottom);
  FLATTEN_EDGES(Margin)
  FLATTEN_EDGES(Padding)
#undef FLATTEN_EDGES

  const auto children = this->GetDirectChildren();
  for (auto&& child: children) {
    constexpr auto clearFlags
      = StateFlags::ActiveInherited | StateFlags::HoveredInherited;
    child->mStateFlags &= ~clearFlags;
  }

  {
    using enum ComputedStyleFlags;
    if (const auto flags = this->OnComputedStyleChange(style);
        flags != Default) {
      auto stateFlags = StateFlags::Default;
      auto stateFlagValues = StateFlags::Default;

      if ((flags & InheritableActiveState) == InheritableActiveState) {
        stateFlags |= StateFlags::ActiveInherited;
        if (isActive) {
          stateFlagValues |= StateFlags::ActiveInherited;
        }
      }
      if ((flags & InheritableHoverState) == InheritableHoverState) {
        stateFlags |= StateFlags::HoveredInherited;
        if (isHovered) {
          stateFlagValues |= StateFlags::HoveredInherited;
        }
      }

      for (auto&& child: this->GetDirectChildren()) {
        child->mStateFlags &= ~stateFlags;
        child->mStateFlags |= stateFlagValues;
      }
    }
  }

  if (mComputedStyle != Style {}) {
    mStyleTransitions->Apply(mComputedStyle, &style);
  }

  mInheritedStyles = inherited;
  mComputedStyle = style;

  const auto childStyles = merged.InheritableStyles();
  for (auto&& child: this->GetDirectChildren()) {
    child->ComputeStyles(childStyles);
  }

  const auto yoga = this->GetLayoutNode();
  const auto setYoga = [&]<class... FrontArgs>(
                         auto member, auto setter, FrontArgs&&... frontArgs) {
    const auto& optional = mComputedStyle.*member;
    if (optional.has_value()) {
      setter(yoga, std::forward<FrontArgs>(frontArgs)..., optional.value());
      return;
    }

    using T = typename std::decay_t<decltype(optional)>::value_type;
    using default_t = yoga_default_value_t<T>;
    if constexpr (requires { default_t::value; }) {
      setter(yoga, std::forward<FrontArgs>(frontArgs)..., default_t::value);
    }
  };

  setYoga(&Style::mAlignItems, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mAlignSelf, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mBottom, &YGNodeStyleSetPosition, YGEdgeBottom);
  setYoga(&Style::mDisplay, &YGNodeStyleSetDisplay);
  setYoga(&Style::mFlexDirection, &YGNodeStyleSetFlexDirection);
  setYoga(&Style::mGap, &YGNodeStyleSetGap, YGGutterAll);
  setYoga(&Style::mHeight, &YGNodeStyleSetHeight);
  setYoga(&Style::mLeft, &YGNodeStyleSetPosition, YGEdgeLeft);
  setYoga(&Style::mMarginBottom, &YGNodeStyleSetMargin, YGEdgeBottom);
  setYoga(&Style::mMarginLeft, &YGNodeStyleSetMargin, YGEdgeLeft);
  setYoga(&Style::mMarginRight, &YGNodeStyleSetMargin, YGEdgeRight);
  setYoga(&Style::mMarginTop, &YGNodeStyleSetMargin, YGEdgeTop);
  setYoga(&Style::mPaddingBottom, &YGNodeStyleSetPadding, YGEdgeBottom);
  setYoga(&Style::mPaddingLeft, &YGNodeStyleSetPadding, YGEdgeLeft);
  setYoga(&Style::mPaddingRight, &YGNodeStyleSetPadding, YGEdgeRight);
  setYoga(&Style::mPaddingTop, &YGNodeStyleSetPadding, YGEdgeTop);
  setYoga(&Style::mRight, &YGNodeStyleSetPosition, YGEdgeRight);
  setYoga(&Style::mTop, &YGNodeStyleSetPosition, YGEdgeTop);
  setYoga(&Style::mWidth, &YGNodeStyleSetWidth);
}

Widget::ComputedStyleFlags Widget::OnComputedStyleChange(const Style&) {
  auto ret = ComputedStyleFlags::Default;
  if ((mStateFlags & StateFlags::HoveredInherited) != StateFlags::Default) {
    ret |= ComputedStyleFlags::InheritableHoverState;
  }
  if ((mStateFlags & StateFlags::ActiveInherited) != StateFlags::Default) {
    ret |= ComputedStyleFlags::InheritableActiveState;
  }
  return ret;
}
}// namespace FredEmmott::GUI::Widgets