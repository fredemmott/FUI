// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/FocusManager.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <felly/overload.hpp>

#include "Widget.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace widget_detail;

namespace {
template <style_detail::StylePropertyKey P>
constexpr auto default_v = style_detail::default_property_value_v<P>;

constexpr auto MakeYogaValue(const auto v) {
  return v;
}

constexpr auto MakeYogaValue(const Overflow overflow) {
  static_assert(std::to_underlying(Overflow::Hidden) == YGOverflowHidden);
  static_assert(std::to_underlying(Overflow::Visible) == YGOverflowVisible);
  static_assert(std::to_underlying(Overflow::Scroll) == YGOverflowScroll);
  return static_cast<YGOverflow>(overflow);
}

constexpr auto MakeYogaValue(const Display display) {
  static_assert(std::to_underlying(Display::None) == YGDisplayNone);
  static_assert(std::to_underlying(Display::Flex) == YGDisplayFlex);
  static_assert(std::to_underlying(Display::Contents) == YGDisplayContents);
  return static_cast<YGDisplay>(display);
}

constexpr auto MakeYogaValue(const FlexDirection v) {
  static_assert(std::to_underlying(FlexDirection::Row) == YGFlexDirectionRow);
  static_assert(
    std::to_underlying(FlexDirection::Column) == YGFlexDirectionColumn);
  static_assert(
    std::to_underlying(FlexDirection::RowReverse) == YGFlexDirectionRowReverse);
  static_assert(
    std::to_underlying(FlexDirection::ColumnReverse)
    == YGFlexDirectionColumnReverse);
  return static_cast<YGFlexDirection>(v);
}

constexpr auto MakeYogaValue(const Align v) {
  static_assert(std::to_underlying(Align::Auto) == YGAlignAuto);
  static_assert(std::to_underlying(Align::FlexStart) == YGAlignFlexStart);
  static_assert(std::to_underlying(Align::Center) == YGAlignCenter);
  static_assert(std::to_underlying(Align::FlexEnd) == YGAlignFlexEnd);
  static_assert(std::to_underlying(Align::Stretch) == YGAlignStretch);
  static_assert(std::to_underlying(Align::Baseline) == YGAlignBaseline);
  static_assert(std::to_underlying(Align::SpaceBetween) == YGAlignSpaceBetween);
  static_assert(std::to_underlying(Align::SpaceAround) == YGAlignSpaceAround);
  static_assert(std::to_underlying(Align::SpaceEvenly) == YGAlignSpaceEvenly);
  return static_cast<YGAlign>(v);
}

constexpr auto MakeYogaValue(const BoxSizing v) {
  static_assert(
    std::to_underlying(BoxSizing::ContentBox) == YGBoxSizingContentBox);
  static_assert(
    std::to_underlying(BoxSizing::BorderBox) == YGBoxSizingBorderBox);
  return static_cast<YGBoxSizing>(v);
}

constexpr auto MakeYogaValue(const Justify v) {
  static_assert(std::to_underlying(Justify::FlexStart) == YGJustifyFlexStart);
  static_assert(std::to_underlying(Justify::Center) == YGJustifyCenter);
  static_assert(std::to_underlying(Justify::FlexEnd) == YGJustifyFlexEnd);
  static_assert(
    std::to_underlying(Justify::SpaceBetween) == YGJustifySpaceBetween);
  static_assert(
    std::to_underlying(Justify::SpaceAround) == YGJustifySpaceAround);
  static_assert(
    std::to_underlying(Justify::SpaceEvenly) == YGJustifySpaceEvenly);
  return static_cast<YGJustify>(v);
}

}// namespace

void Widget::ComputeStyles(const Style& inherited) {
  static const auto GlobalBaselineStyle = Style::BuiltinBaseline();

  const auto fm = this->GetOwnerWindow()->GetFocusManager();
  FUI_ASSERT(fm);
  mDirectStateFlags &= ~(StateFlags::HaveFocus | StateFlags::HaveVisibleFocus);
  if (const auto target = fm->GetFocusedWidget()) {
    if (const auto [widget, reason] = *target; widget == this) {
      mDirectStateFlags |= StateFlags::HaveFocus;
      if (reason == FocusKind::Visible) {
        mDirectStateFlags |= StateFlags::HaveVisibleFocus;
      }
    }
  }

  if (mStylesCacheKey.empty()) {
    mStylesCacheKey.resize(sizeof(void*) * (mClassList.size() + 1));
    const auto cacheKeyPointers
      = reinterpret_cast<uintptr_t*>(mStylesCacheKey.data());
    std::ranges::copy(
      mClassList | std::views::transform(&StyleClass::AsCacheKey),
      cacheKeyPointers + 1);
  }
  reinterpret_cast<uintptr_t*>(mStylesCacheKey.data())[0]
    = static_cast<uintptr_t>(mDirectStateFlags | mInheritedStateFlags);

  auto flattened = mImmutableStyle.GetCached(mStylesCacheKey);
  if (!flattened) {
    if (mImmutableStyle) {
      flattened = FlattenStyles(GlobalBaselineStyle + mImmutableStyle.Get());
      mImmutableStyle.EmplaceCache(mStylesCacheKey, *flattened);
    } else {
      flattened = GlobalBaselineStyle;
    }
  }

  auto style = flattened.value() + inherited + mMutableStyles;

  mDirectStateFlags &= ~StateFlags::Animating;

  style = FlattenStyles(style);

  for (auto&& child: mRawStructuralChildren) {
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
      for (auto&& child: mRawStructuralChildren) {
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
  for (auto&& child: mRawStructuralChildren) {
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
          setter(
            yoga,
            std::forward<FrontArgs>(frontArgs)...,
            MakeYogaValue(optional.value()));
          return;
        }

        if constexpr (!std::same_as<
                        std::nullopt_t,
                        std::decay_t<decltype(defaultValue)>>) {
          setter(
            yoga,
            std::forward<FrontArgs>(frontArgs)...,
            MakeYogaValue(defaultValue));
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
  X(AspectRatio, AspectRatio)
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

  if (it == PseudoClasses::Focus) {
    return (state & StateFlags::HaveFocus) != StateFlags::Default;
  }

  if (it == PseudoClasses::FocusVisible) {
    return (state & StateFlags::HaveVisibleFocus) != StateFlags::Default;
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
  return std::visit(
    felly::overload {
      [this](const Widget* it) { return it == this; },
      [this](const StyleClass& it) { return MatchesStyleClass(it); },
      [this](const NegatedStyleClass& it) {
        return !MatchesStyleClass(it.mStyleClass);
      },
      [](std::monostate) { return true; },
    },
    selector);
}

}// namespace FredEmmott::GUI::Widgets