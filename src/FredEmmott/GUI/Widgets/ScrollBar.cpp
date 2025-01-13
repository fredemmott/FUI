// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBar.hpp"

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

ScrollBar::ScrollBar(std::size_t id, Orientation orientation)
  : Widget(id),
    mOrientation(orientation),
    mBuiltinStyles(GetBuiltinStylesForOrientation()) {
  using namespace StaticTheme::ScrollBar;
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  constexpr auto leftGlyph = "\uedd9";
  constexpr auto rightGlyph = "\uedda";
  constexpr auto upGlyph = "\ueddb";
  constexpr auto downGlyph = "\ueddc";

  this->ChangeDirectChildren([this] {
    mSmallDecrement.reset(new Label(0));
    mLargeDecrement.reset(new Widget(0));
    mThumb.reset(new Widget(0));
    mLargeIncrement.reset(new Widget(0));
    mSmallIncrement.reset(new Label(0));
  });

  const auto smallChangeOpacityAnimation = CubicBezierStyleTransition(
    ScrollBarContractBeginTime,
    ScrollBarOpacityChangeDuration,
    StaticTheme::Common::ControlFastOutSlowInKeySpline);

  static const WidgetStyles sSmallChangeStyles {
    .mBase = {
      .mColor = ScrollBarButtonArrowForeground,
      .mFont = {
        ResolveGlyphFont(SystemFont::Body).WithSizeInPixels(ScrollBarButtonArrowIconFontSize),
        !important,
      },
      .mOpacity = { 0, smallChangeOpacityAnimation },
    },
    .mHover = {
      .mColor = ScrollBarButtonArrowForegroundPointerOver,
    },
    .mActive = {
      .mColor = ScrollBarButtonArrowForegroundPressed,
    },
  };

  mSmallDecrement->SetBuiltInStyles(sSmallChangeStyles);
  mSmallIncrement->SetBuiltInStyles(sSmallChangeStyles);

  static const WidgetStyles sLargeChangeStyles {
    .mBase = {
      .mFlexGrow = 1,
    },
  };
  mLargeDecrement->SetBuiltInStyles(sLargeChangeStyles);
  mLargeIncrement->SetBuiltInStyles(sLargeChangeStyles);

  auto thumbStyles  = WidgetStyles {
    .mBase = Style {
      .mBackgroundColor = ScrollBarThumbFill,
      .mBorderRadius = ScrollBarCornerRadius,
    },
    .mDisabled = Style {
      .mBackgroundColor = ScrollBarThumbFillDisabled,
    },
    .mHover = Style {
      .mBackgroundColor = ScrollBarThumbFillPointerOver,
    },
    .mActive = Style {
      .mBackgroundColor = ScrollBarThumbFillPressed,
    },
  };
  switch (orientation) {
    case Orientation::Vertical:
      thumbStyles.mBase += Style {
        .mMinHeight = ScrollBarVerticalThumbMinHeight,
        .mMinWidth = ScrollBarVerticalThumbMinWidth,
      };
      mSmallDecrement->SetText(upGlyph);
      mSmallIncrement->SetText(downGlyph);
      break;
    case Orientation::Horizontal:
      thumbStyles.mBase += Style {
        .mMinHeight = ScrollBarHorizontalThumbMinHeight,
        .mMinWidth = ScrollBarHorizontalThumbMinWidth,
      };
      mSmallDecrement->SetText(leftGlyph);
      mSmallIncrement->SetText(rightGlyph);
      break;
  }

  mThumb->SetBuiltInStyles(thumbStyles);
}

ScrollBar::~ScrollBar() = default;

WidgetList ScrollBar::GetDirectChildren() const noexcept {
  return {
    mSmallDecrement.get(),
    mLargeDecrement.get(),
    mThumb.get(),
    mLargeIncrement.get(),
    mSmallIncrement.get(),
  };
}
WidgetStyles ScrollBar::GetBuiltInStyles() const {
  return mBuiltinStyles;
}

Widget::ComputedStyleFlags ScrollBar::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  const bool hovered = (state & StateFlags::Hovered) == StateFlags::Hovered;

  const WidgetStyles smallChangeStyles {
    .mBase = {
      .mOpacity = (hovered ? 1 : 0),
    },
  };
  mSmallDecrement->SetExplicitStyles(smallChangeStyles);
  mSmallIncrement->SetExplicitStyles(smallChangeStyles);

  return Widget::OnComputedStyleChange(style, state);
}

WidgetStyles ScrollBar::GetBuiltinStylesForOrientation() const {
  using namespace StaticTheme::ScrollBar;

  static const WidgetStyles sBaseStyles {
    .mBase = {
      .mBackgroundColor = ScrollBarBackground,
    },
    .mDisabled = {
      .mBackgroundColor = ScrollBarBackgroundDisabled,
    },
    .mHover = {
      .mBackgroundColor = ScrollBarBackgroundPointerOver,
    },
  };
  static const WidgetStyles sHorizontalStyles = sBaseStyles + WidgetStyles {
    .mBase = {
      .mFlexDirection = YGFlexDirectionRow,
    },
  };
  static const WidgetStyles sVerticalStyles = sBaseStyles + WidgetStyles {
    .mBase = {
      .mFlexDirection = YGFlexDirectionColumn,
    },
  };

  if (mOrientation == Orientation::Horizontal) {
    return sHorizontalStyles;
  } else {
    return sVerticalStyles;
  }
}

}// namespace FredEmmott::GUI::Widgets