// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/Widgets/PopupWindow.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {
using Widgets::PopupWindow;
using namespace immediate_detail;

namespace {

struct ParentContext {
  Window* mPreviousWindow {nullptr};
  Window* mWindow {nullptr};
  decltype(tStack) mWindowStack;
  ActivatedFlag mNeedAdditionalFrame;
};
thread_local std::vector<ParentContext> tPopupStack;

void PopParentContext() {
  auto& back = tPopupStack.back();
  tWindow = back.mPreviousWindow;
  tStack = std::move(back.mWindowStack);

  const bool popupNeedsFrame = tNeedAdditionalFrame.TestAndClear();

  tNeedAdditionalFrame = back.mNeedAdditionalFrame;
  if (popupNeedsFrame) {
    tNeedAdditionalFrame.Set();
  }

  tPopupStack.pop_back();
}

}// namespace

void PopupWindowResultMixin::MakeTransparent(const bool transparent) {
  static_cast<Win32Window*>(tWindow)->SetSystemBackdropType(
    transparent ? DWMSBT_NONE : DWMSBT_TRANSIENTWINDOW);
}

BasicPopupWindowResult<&EndBasicPopupWindow> BeginBasicPopupWindow(
  const ID id) {
  auto anchor = GetCurrentNode();

  BeginWidget<PopupWindow>(id);
  auto window = GetCurrentParentNode<PopupWindow>()->GetWindow();
  window->SetParent(tWindow->GetNativeHandle());
  if (anchor && !window->GetNativeHandle()) {
    if (const auto ctx = anchor->GetContext<PopupAnchorContext>()) {
      anchor = ctx->mAnchor;
    }
    window->SetInitialPositionInNativeCoords(
      tWindow->CanvasPointToNativePoint(anchor->GetTopLeftCanvasPoint()));
  }

  tPopupStack.emplace_back(
    tWindow, window, std::move(tStack), tNeedAdditionalFrame);
  tWindow = nullptr;
  tStack = {};

  // TODO: mark as closed, handle re-open
  if (window->BeginFrame()) {
    return true;
  }

  PopParentContext();
  EndWidget<PopupWindow>();
  return false;
}

BasicPopupWindowResult<&EndBasicPopupWindow> BeginBasicPopupWindow(
  bool* open,
  ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginBasicPopupWindow(id);
  return *open;
}
void EndBasicPopupWindow() {
  auto window = tPopupStack.back().mWindow;
  window->EndFrame();
  PopParentContext();
  EndWidget<PopupWindow>();
}

void EndPopupWindow() {
  EndWidget<Widget>();
  EndBasicPopupWindow();
}

PopupWindowResult BeginPopupWindow(const ID id) {
  if (!BeginBasicPopupWindow(id).Transparent()) {
    return false;
  }

  const auto widget = BeginWidget<Widget>(ID {0});

  using namespace StaticTheme::Common;
  widget->SetBuiltInStyles({
    .mBackgroundColor = AcrylicBackgroundFillColorDefaultBrush,
    .mBorderColor = SurfaceStrokeColorDefaultBrush,
    .mBorderRadius = OverlayCornerRadius,
    .mBorderWidth = 2,
    .mPadding = 20,
  });

  return {true};
}

PopupWindowResult BeginPopupWindow(bool* open, ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginPopupWindow(id);
  return *open;
}

}// namespace FredEmmott::GUI::Immediate