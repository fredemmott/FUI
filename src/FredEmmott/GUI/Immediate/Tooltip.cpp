// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Tooltip.hpp"

#include <stdexcept>
#include <utility>

#include "Card.hpp"
#include "FredEmmott/GUI/StaticTheme/ToolTip.hpp"
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "PopupWindow.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {

struct TooltipContext : Widgets::Context {
  bool mVisible {false};
};

}// namespace

void EndTooltip() {
  immediate_detail::EndWidget();
  EndBasicPopupWindow();
}

[[nodiscard]]
TooltipResult BeginTooltipForPreviousWidget(const ID id) {
  using namespace immediate_detail;
  const auto w = GetCurrentNode();
  if (!w) [[unlikely]] {
    throw std::logic_error("No previous sibling widget");
  }

  const auto ctx = w->GetOrCreateContext<TooltipContext>();
  const bool wasVisible = ctx->mVisible;
  if (!w->IsHovered()) {
    ctx->mVisible = false;
    return false;
  }

  if (std::exchange(w->mWasStationaryHovered, false)) {
    ctx->mVisible = true;
  }

  if (!ctx->mVisible) {
    return false;
  }

  ctx->mVisible = BeginBasicPopupWindow(id).Transparent();
  if (!ctx->mVisible) {
    return false;
  }

  if (!wasVisible) {
    const auto window = static_cast<Win32Window*>(tWindow);
    window->MutateStyles(
      []([[maybe_unused]] DWORD* styles, DWORD* extendedStyles) {
        *extendedStyles |= WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW
          | WS_EX_TRANSPARENT | WS_EX_LAYERED;
      });
  }
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ToolTip;
  static const ImmutableStyle TooltipStyles {
    Style()
      .BackgroundColor(ToolTipBackgroundBrush)
      .BorderColor(ToolTipBorderBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(ToolTipBorderThemeThickness)
      .FlexDirection(YGFlexDirectionColumn)
      .Font(
        SystemFont::Resolve(SystemFont::Body)
          .WithSize(ToolTipContentThemeFontSize))
      .Color(ToolTipForegroundBrush)
      .MaxWidth(ToolTipMaxWidth)
      .PaddingLeft(ToolTipBorderPaddingLeft)
      .PaddingTop(ToolTipBorderPaddingTop)
      .PaddingBottom(ToolTipBorderPaddingBottom)
      .PaddingRight(ToolTipBorderPaddingRight),
  };
  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"Tooltip/Root"}, TooltipStyles);
  return true;
}
}// namespace FredEmmott::GUI::Immediate