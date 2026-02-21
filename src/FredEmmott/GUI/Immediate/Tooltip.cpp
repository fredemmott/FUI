// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Tooltip.hpp"

#include <stdexcept>
#include <utility>

#include "Card.hpp"
#include "FredEmmott/GUI/StaticTheme/Common.hpp"
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
  static const ImmutableStyle TooltipStyles {
    Style()
      .BackgroundColor(AcrylicBackgroundFillColorDefaultBrush)
      .BorderColor(SurfaceStrokeColorDefaultBrush)
      .BorderRadius(OverlayCornerRadius)
      .BorderWidth(2)
      .FlexDirection(YGFlexDirectionColumn)
      .Padding(20),
  };
  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"Tooltip/Root"}, TooltipStyles);
  return true;
}
}// namespace FredEmmott::GUI::Immediate