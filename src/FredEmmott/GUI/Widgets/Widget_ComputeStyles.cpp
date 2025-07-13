// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "Widget.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace widget_detail;

namespace {
template <style_detail::StyleProperty P>
constexpr auto default_v = style_detail::default_property_value_v<P>;
}

void Widget::ComputeStyles(const Style& inherited) {
  const auto clean = wil::scope_exit([this] { mDirtyStyles = false; });
  static const auto GlobalBaselineStyle = Style::BuiltinBaseline();

  Style style = GlobalBaselineStyle
    + (mReplacedBuiltInStyles ? mReplacedBuiltInStyles.value()
                              : this->GetBuiltInStyles());
  style += inherited;
  style += mExplicitStyles;

  mDirectStateFlags &= ~StateFlags::Animating;
  const auto stateFlags = mDirectStateFlags | mInheritedStateFlags;

  const bool isHovered
    = (stateFlags & StateFlags::Hovered) != StateFlags::Default;
  const bool isActive
    = (stateFlags & StateFlags::Active) != StateFlags::Default;
  const bool isDisabled
    = (stateFlags & StateFlags::Disabled) != StateFlags::Default;

  bool haveChanges = false;
  do {
    haveChanges = false;
    for (auto&& [selector, rules]: style.mAnd) {
      if (this->MatchesStyleSelector(selector)) {
        style += rules;
        haveChanges = true;
      }
    }
    style.mAnd.clear();
  } while (haveChanges);

  const auto flattenEdge = [&style]<class T>(T allEdges, T thisEdge) {
    auto& thisEdgeOpt = style.*thisEdge;
    const auto& allEdgesOpt = style.*allEdges;

    thisEdgeOpt = allEdgesOpt + thisEdgeOpt;
  };
  const auto flattenEdges
    = [&flattenEdge]<class T, std::same_as<T>... TRest>(T all, TRest... rest) {
        (flattenEdge(all, rest), ...);
      };
#define FLATTEN_EDGES(X, Y) \
  flattenEdges( \
    &Style::m##X##Y, \
    &Style::m##X##Left##Y, \
    &Style::m##X##Top##Y, \
    &Style::m##X##Right##Y, \
    &Style::m##X##Bottom##Y);
  FUI_STYLE_EDGE_PROPERTIES(FLATTEN_EDGES)
#undef FLATTEN_EDGES

  const auto children = this->GetDirectChildren();
  for (auto&& child: children) {
    child->mInheritedStateFlags = {};
  }

  {
    using enum ComputedStyleFlags;
    auto propagateFlags = StateFlags::Default;
    if (isDisabled) {
      propagateFlags |= StateFlags::Disabled;
    }
    if (const auto flags = this->OnComputedStyleChange(
          style, mDirectStateFlags | mInheritedStateFlags);
        flags != Empty) {
      if ((flags & InheritableActiveState) == InheritableActiveState) {
        if (isActive) {
          propagateFlags |= StateFlags::Active;
        }
      }
      if ((flags & InheritableHoverState) == InheritableHoverState) {
        if (isHovered) {
          propagateFlags |= StateFlags::Hovered;
        }
      }
      if ((flags & Animating) == Animating) {
        mDirectStateFlags |= StateFlags::Animating;
      }
    }

    if (propagateFlags != StateFlags::Default) {
      for (auto&& child: this->GetDirectChildren()) {
        child->mInheritedStateFlags = propagateFlags;
      }
    }
  }

  if (mComputedStyle != Style {}) {
    if (
      mStyleTransitions->Apply(mComputedStyle, &style)
      == StyleTransitions::ApplyResult::Animating) {
      mDirectStateFlags |= StateFlags::Animating;
    }
  }

  mInheritedStyles = inherited;
  mComputedStyle = style;

  const auto childStyles = style.InheritableValues();
  for (auto&& child: this->GetDirectChildren()) {
    child->ComputeStyles(childStyles);
  }

  const auto yoga = this->GetLayoutNode();
  const auto setYoga = [&]<class... FrontArgs>(
                         const auto defaultValue,
                         auto member,
                         auto setter,
                         FrontArgs&&... frontArgs) {
    const auto& optional = mComputedStyle.*member;
    if (optional.has_value()) {
      setter(yoga, std::forward<FrontArgs>(frontArgs)..., optional.value());
      return;
    }

    if constexpr (!std::same_as<
                    std::nullopt_t,
                    std::decay_t<decltype(defaultValue)>>) {
      setter(yoga, std::forward<FrontArgs>(frontArgs)..., defaultValue);
    }
  };

  using enum style_detail::StyleProperty;
  setYoga(default_v<Position>, &Style::mPosition, &YGNodeStyleSetPositionType);

  setYoga(
    default_v<AlignContent>,
    &Style::mAlignContent,
    &YGNodeStyleSetAlignContent);
  setYoga(
    default_v<AlignItems>, &Style::mAlignItems, &YGNodeStyleSetAlignItems);
  setYoga(default_v<AlignSelf>, &Style::mAlignSelf, &YGNodeStyleSetAlignSelf);
  setYoga(
    default_v<BorderBottomWidth>,
    &Style::mBorderBottomWidth,
    &YGNodeStyleSetBorder,
    YGEdgeBottom);
  setYoga(
    default_v<BorderLeftWidth>,
    &Style::mBorderLeftWidth,
    &YGNodeStyleSetBorder,
    YGEdgeLeft);
  setYoga(
    default_v<BorderRightWidth>,
    &Style::mBorderRightWidth,
    &YGNodeStyleSetBorder,
    YGEdgeRight);
  setYoga(
    default_v<BorderTopWidth>,
    &Style::mBorderTopWidth,
    &YGNodeStyleSetBorder,
    YGEdgeTop);
  setYoga(
    default_v<Bottom>, &Style::mBottom, &YGNodeStyleSetPosition, YGEdgeBottom);
  setYoga(default_v<Display>, &Style::mDisplay, &YGNodeStyleSetDisplay);
  setYoga(default_v<FlexBasis>, &Style::mFlexBasis, &YGNodeStyleSetFlexBasis);
  setYoga(
    default_v<FlexDirection>,
    &Style::mFlexDirection,
    &YGNodeStyleSetFlexDirection);
  setYoga(default_v<FlexGrow>, &Style::mFlexGrow, &YGNodeStyleSetFlexGrow);
  setYoga(
    default_v<FlexShrink>, &Style::mFlexShrink, &YGNodeStyleSetFlexShrink);
  setYoga(default_v<Gap>, &Style::mGap, &YGNodeStyleSetGap, YGGutterAll);
  setYoga(default_v<Height>, &Style::mHeight, &YGNodeStyleSetHeight);
  setYoga(
    default_v<JustifyContent>,
    &Style::mJustifyContent,
    &YGNodeStyleSetJustifyContent);
  setYoga(default_v<Left>, &Style::mLeft, &YGNodeStyleSetPosition, YGEdgeLeft);
  setYoga(
    default_v<MarginBottom>,
    &Style::mMarginBottom,
    &YGNodeStyleSetMargin,
    YGEdgeBottom);
  setYoga(
    default_v<MarginLeft>,
    &Style::mMarginLeft,
    &YGNodeStyleSetMargin,
    YGEdgeLeft);
  setYoga(
    default_v<MarginRight>,
    &Style::mMarginRight,
    &YGNodeStyleSetMargin,
    YGEdgeRight);
  setYoga(
    default_v<MarginTop>, &Style::mMarginTop, &YGNodeStyleSetMargin, YGEdgeTop);
  setYoga(default_v<MaxHeight>, &Style::mMaxHeight, &YGNodeStyleSetMaxHeight);
  setYoga(default_v<MaxWidth>, &Style::mMaxWidth, &YGNodeStyleSetMaxWidth);
  setYoga(default_v<MinHeight>, &Style::mMinHeight, &YGNodeStyleSetMinHeight);
  setYoga(default_v<MinWidth>, &Style::mMinWidth, &YGNodeStyleSetMinWidth);
  setYoga(default_v<Overflow>, &Style::mOverflow, &YGNodeStyleSetOverflow);
  setYoga(
    default_v<PaddingBottom>,
    &Style::mPaddingBottom,
    &YGNodeStyleSetPadding,
    YGEdgeBottom);
  setYoga(
    default_v<PaddingLeft>,
    &Style::mPaddingLeft,
    &YGNodeStyleSetPadding,
    YGEdgeLeft);
  setYoga(
    default_v<PaddingRight>,
    &Style::mPaddingRight,
    &YGNodeStyleSetPadding,
    YGEdgeRight);
  setYoga(
    default_v<PaddingTop>,
    &Style::mPaddingTop,
    &YGNodeStyleSetPadding,
    YGEdgeTop);
  setYoga(
    default_v<Right>, &Style::mRight, &YGNodeStyleSetPosition, YGEdgeRight);
  setYoga(default_v<Top>, &Style::mTop, &YGNodeStyleSetPosition, YGEdgeTop);
  setYoga(default_v<Width>, &Style::mWidth, &YGNodeStyleSetWidth);
}

Widget::ComputedStyleFlags Widget::OnComputedStyleChange(
  const Style&,
  StateFlags) {
  auto ret = ComputedStyleFlags::Empty;
  if ((mInheritedStateFlags & StateFlags::Hovered) != StateFlags::Default) {
    ret |= ComputedStyleFlags::InheritableHoverState;
  }
  if ((mInheritedStateFlags & StateFlags::Active) != StateFlags::Default) {
    ret |= ComputedStyleFlags::InheritableActiveState;
  }
  return ret;
}

bool Widget::MatchesStylePseudoClass(const StyleClass it) const {
  const auto state = mDirectStateFlags | mInheritedStateFlags;
  if ((state & StateFlags::Disabled) != StateFlags::Default) {
    return it == PseudoClasses::Disabled;
  }

  if (it == PseudoClasses::Hover) {
    return (state & (StateFlags::Hovered | StateFlags::Active))
      != StateFlags::Default;
  }

  if (it == PseudoClasses::Active) {
    return (state & StateFlags::Active) != StateFlags::Default;
  }

  return false;
}

bool Widget::MatchesStyleSelector(Style::Selector selector) const {
  if (const auto it = get_if<const Widget*>(&selector)) {
    return *it == this;
  }
  if (const auto it = get_if<StyleClass>(&selector)) {
    if (mClassList.contains(*it)) {
      return true;
    }
    return MatchesStylePseudoClass(*it);
  }
  if (holds_alternative<std::monostate>(selector)) {
    return true;
  }
#ifndef NDEBUG
  __debugbreak();
#endif
  return false;
}

}// namespace FredEmmott::GUI::Widgets