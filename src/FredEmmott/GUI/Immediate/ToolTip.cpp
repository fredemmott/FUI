// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ToolTip.hpp"

#include <FredEmmott/GUI/yoga.hpp>
#include <stdexcept>
#include <utility>

#include "Card.hpp"
#include "FredEmmott/GUI/StaticTheme/ToolTip.hpp"
#include "FredEmmott/GUI/Widgets/PopupWindow.hpp"
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "PopupWindow.hpp"
namespace FredEmmott::GUI::Immediate {
namespace {

struct ToolTipAnchorContext : Widgets::Context {
  Window* mParentWindow {nullptr};
  std::optional<Point> mAnchorTo;
  bool mVisible {false};
};

struct ToolTipContainerContext : Widgets::Context {
  Widgets::Widget* mAnchor {};
};

}// namespace

void EndToolTip() {
  using namespace immediate_detail;
  auto p = GetCurrentParentNode();
  EndWidget();

  const auto ctx = p->GetContext<ToolTipContainerContext>()
                     ->mAnchor->GetContext<ToolTipAnchorContext>();
  if (const auto cursorPoint = std::exchange(ctx->mAnchorTo, std::nullopt)) {
    p->ComputeStyles({});
    const auto [w, h] = GetMinimumWidthAndIdealHeight(p->GetLayoutNode());

    static constexpr Point FixedOffset {0, -12};
    const Point contentOffset {-w / 2, -h};
    const auto point = *cursorPoint + FixedOffset + contentOffset;
    const auto nativePoint
      = ctx->mParentWindow->CanvasPointToNativePoint(point);
    tWindow->SetInitialPositionInNativeCoords(nativePoint);
  }

  EndBasicPopupWindow();
}

[[nodiscard]]
ToolTipResult BeginToolTipForPreviousWidget(const ID id) {
  using namespace immediate_detail;
  return BeginToolTipForWidget(GetCurrentNode(), id);
}

namespace immediate_detail {
[[nodiscard]]
ToolTipResult BeginToolTipForWidget(Widgets::Widget* w, const ID id) {
  if (!w) [[unlikely]] {
    throw std::logic_error("No previous sibling widget");
  }

  const auto ctx = w->GetOrCreateContext<ToolTipAnchorContext>();
  const bool wasVisible = ctx->mVisible;
  if (!w->IsHovered()) {
    ctx->mVisible = false;
    return false;
  }

  const auto hoverEvent = std::exchange(w->mWasStationaryHovered, std::nullopt);
  if (hoverEvent) {
    ctx->mParentWindow = tWindow;
    ctx->mVisible = true;
    if (!wasVisible) {
      ctx->mAnchorTo = hoverEvent->mWindowPoint;
    }
  }

  if (!ctx->mVisible) {
    return false;
  }

  ctx->mVisible = BeginBasicPopupWindow(id).Transparent();
  if (!ctx->mVisible) {
    return false;
  }

  if (!wasVisible) {
    tWindow->SetIsToolTip();
  }
  const auto container = BeginWidget<Widget>(
    ID {0},
    LiteralStyleClass {"ToolTip/Root"},
    StaticTheme::ToolTip::DefaultToolTipStyle());
  if (!wasVisible) {
    container->GetOrCreateContext<ToolTipContainerContext>()->mAnchor = w;
  }
  return true;
}
}// namespace immediate_detail
}// namespace FredEmmott::GUI::Immediate