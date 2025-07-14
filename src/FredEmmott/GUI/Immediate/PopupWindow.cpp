// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/Widgets/PopupWindow.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "EnqueueAdditionalFrame.hpp"
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

void BasicPopupWindowResultMixin::MakeTransparent(const bool transparent) {
  static_cast<Win32Window*>(tWindow)->SetSystemBackdropType(
    transparent ? DWMSBT_NONE : DWMSBT_TRANSIENTWINDOW);
}
void BasicPopupWindowResultMixin::MakeModal(bool modal) {
  static_cast<Win32Window*>(tWindow)->SetIsModal(modal);
}

BasicPopupWindowResult BeginBasicPopupWindow(const ID id) {
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
  tStack.back().mNewSiblings.pop_back();
  return false;
}

BasicPopupWindowResult BeginBasicPopupWindow(bool* open, ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginBasicPopupWindow(id);
  return *open;
}

void ClosePopupWindow() {
  tWindow->RequestStop(EXIT_SUCCESS);
}

void EndBasicPopupWindow() {
  auto window = tPopupStack.back().mWindow;
  window->EndFrame();
  PopParentContext();
  EndWidget<PopupWindow>();
}

void EndPopup() {
  EndWidget<Widget>();
  EndBasicPopupWindow();
}

PopupResult BeginPopup(const ID id) {
  if (!BeginBasicPopupWindow(id).Transparent()) {
    return false;
  }

  const auto widget = BeginWidget<Widget>(ID {0});

  using namespace StaticTheme::Common;
  widget->SetBuiltInStyles(
    Style()
      .BackgroundColor(AcrylicBackgroundFillColorDefaultBrush)
      .BorderColor(SurfaceStrokeColorDefaultBrush)
      .BorderRadius(OverlayCornerRadius)
      .BorderWidth(2)
      .Padding(20));

  return {true};
}

PopupResult BeginPopup(bool* open, ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginPopup(id);
  return *open;
}

}// namespace FredEmmott::GUI::Immediate