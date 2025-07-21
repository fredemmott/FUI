// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "FredEmmott/GUI/assert.hpp"
#include "Widget.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace widget_detail;

namespace {
template <style_detail::StylePropertyKey P>
constexpr auto default_v = style_detail::default_property_value_v<P>;
}

void Widget::ComputeStyles(const Style& inherited) {
  static const auto GlobalBaselineStyle = Style::BuiltinBaseline();

  std::string cacheKey;
  cacheKey.resize(sizeof(void*) * (mClassList.size() + 1));
  const auto cacheKeyPointers = reinterpret_cast<uintptr_t*>(cacheKey.data());
  cacheKeyPointers[0]
    = static_cast<uintptr_t>(mDirectStateFlags | mInheritedStateFlags);
  std::ranges::copy(
    mClassList | std::views::transform(&StyleClass::AsCacheKey),
    cacheKeyPointers + 1);

  auto flattened = mImmutableStyle.GetCached(cacheKey);
  if (!flattened) {
    flattened = FlattenStyles(GlobalBaselineStyle + mImmutableStyle.Get());
    mImmutableStyle.EmplaceCache(cacheKey, *flattened);
  }

  auto style = flattened.value() + inherited + mExplicitStyles;

  mDirectStateFlags &= ~StateFlags::Animating;

  style = FlattenStyles(style);

  const auto flattenEdge = [&style]<class T, class U>(T allEdges, U thisEdge) {
    auto& thisEdgeOpt = std::invoke(thisEdge, style);
    const auto& allEdgesOpt = std::invoke(allEdges, style);

    thisEdgeOpt = allEdgesOpt + thisEdgeOpt;
  };
  const auto flattenEdges
    = [&flattenEdge]<class T, class... TRest>(T all, TRest... rest) {
        (flattenEdge(all, rest), ...);
      };
#define FLATTEN_EDGES(X, Y) \
  flattenEdges( \
    [](auto&& it) -> auto& { return it.X##Y(); }, \
    [](auto&& it) -> auto& { return it.X##Left##Y(); }, \
    [](auto&& it) -> auto& { return it.X##Top##Y(); }, \
    [](auto&& it) -> auto& { return it.X##Right##Y(); }, \
    [](auto&& it) -> auto& { return it.X##Bottom##Y(); });
  FUI_STYLE_EDGE_PROPERTIES(FLATTEN_EDGES)
#undef FLATTEN_EDGES

  const auto children = this->GetDirectChildren();
  for (auto&& child: children) {
    child->mInheritedStateFlags = {};
  }

  {
    const auto stateFlags = mDirectStateFlags | mInheritedStateFlags;
    const bool isHovered
      = (stateFlags & StateFlags::Hovered) != StateFlags::Default;
    const bool isActive
      = (stateFlags & StateFlags::Active) != StateFlags::Default;
    const bool isDisabled
      = (stateFlags & StateFlags::Disabled) != StateFlags::Default;

    using enum ComputedStyleFlags;
    auto propagateFlags = StateFlags::Default;
    if (isDisabled) {
      propagateFlags |= StateFlags::Disabled;
    }
    if (IsChecked()) {
      propagateFlags |= StateFlags::Checked;
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
  const auto setYoga
    = [&]<class... FrontArgs>(
        const auto defaultValue,
        std::invocable<Style&> auto member,
        auto setter,
        FrontArgs&&... frontArgs) {
        const auto& optional = std::invoke(member, mComputedStyle);
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

  using enum style_detail::StylePropertyKey;
#define X(PROPERTY, YG_SETTER, ...) \
  setYoga( \
    default_v<PROPERTY>, \
    [](auto&& it) -> auto& { return it.PROPERTY(); }, \
    &YGNodeStyleSet##YG_SETTER, \
    ##__VA_ARGS__);
  X(Position, PositionType)
  X(AlignContent, AlignContent)
  X(AlignItems, AlignItems)
  X(AlignSelf, AlignSelf)
  X(BorderBottomWidth, Border, YGEdgeBottom)
  X(BorderLeftWidth, Border, YGEdgeLeft)
  X(BorderRightWidth, Border, YGEdgeRight)
  X(BorderTopWidth, Border, YGEdgeTop)
  X(Bottom, Position, YGEdgeBottom)
  X(BoxSizing, BoxSizing)
  X(Display, Display)
  X(FlexBasis, FlexBasis)
  X(FlexDirection, FlexDirection)
  X(FlexGrow, FlexGrow)
  X(FlexShrink, FlexShrink)
  X(Gap, Gap, YGGutterAll)
  X(Height, Height)
  X(JustifyContent, JustifyContent)
  X(Left, Position, YGEdgeLeft)
  X(MarginBottom, Margin, YGEdgeBottom)
  X(MarginLeft, Margin, YGEdgeLeft)
  X(MarginRight, Margin, YGEdgeRight)
  X(MarginTop, Margin, YGEdgeTop)
  X(MaxHeight, MaxHeight)
  X(MaxWidth, MaxWidth)
  X(MinHeight, MinHeight)
  X(MinWidth, MinWidth)
  X(Overflow, Overflow)
  X(PaddingBottom, Padding, YGEdgeBottom)
  X(PaddingLeft, Padding, YGEdgeLeft)
  X(PaddingRight, Padding, YGEdgeRight)
  X(PaddingTop, Padding, YGEdgeTop)
  X(Right, Position, YGEdgeRight)
  X(Top, Position, YGEdgeTop)
  X(Width, Width)
#undef X
}

Style Widget::FlattenStyles(const Style& inputStyle) {
  Style style = inputStyle;
  while (!style.mAnd.empty()) {
    for (auto&& [selector, rules]: std::exchange(style.mAnd, {})) {
      if (this->MatchesStyleSelector(selector)) {
        style += rules;
      }
    }
  }
  return style;
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
  if (it == PseudoClasses::Checked) {
    return IsChecked();
  }

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

bool Widget::MatchesStyleClass(const StyleClass& klass) const {
  if (mClassList.contains(klass)) {
    return true;
  }
  return MatchesStylePseudoClass(klass);
}

bool Widget::MatchesStyleSelector(const Style::Selector selector) const {
  if (const auto it = get_if<const Widget*>(&selector)) {
    return *it == this;
  }
  if (const auto it = get_if<StyleClass>(&selector)) {
    return MatchesStyleClass(*it);
  }
  if (const auto it = get_if<NegatedStyleClass>(&selector)) {
    return !MatchesStyleClass(it->mStyleClass);
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