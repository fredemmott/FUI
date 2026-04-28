// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/Widgets/PopupWindow.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#ifdef _WIN32
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#endif
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {
using Widgets::PopupWindow;
using namespace immediate_detail;

namespace {

struct ParentContext {
  Window* mPreviousWindow {nullptr};
  Window* mWindow {nullptr};
  decltype(tStack) mWindowStack;
  bool mNeedAdditionalFrame {false};
};
thread_local std::vector<ParentContext> tPopupStack;

void PopParentContext() {
  auto& back = tPopupStack.back();
  tWindow = back.mPreviousWindow;
  tStack = std::move(back.mWindowStack);

  if (back.mNeedAdditionalFrame) {
    tNeedAdditionalFrame = true;
  }

  tPopupStack.pop_back();
}

}// namespace

void BasicPopupWindowResultMixin::MakeModal(const bool modal) {
#ifdef _WIN32
  static_cast<Win32Window*>(tWindow)->SetIsModal(modal);
#else
  // Linux: modal popups will be wired through the SDL3 / libdecor
  // window layer (TODO).
  (void)modal;
#endif
}

BasicPopupWindowResult BeginBasicPopupWindow(const ID id) {
  auto anchor = GetCurrentNode();

  BeginWidget<PopupWindow>(id);
  auto window = GetCurrentParentNode<PopupWindow>()->GetWindow();
  if (!window) {
    // Window backend doesn't support popups (e.g. base SdlWindow
    // returns {} from CreatePopup; SdlSkiaVulkanWindow overrides).
    // Treat as immediately-closed: undo the widget push and report
    // "not active" so callers' if-blocks skip the popup body.
    // Mirrors the BeginFrame-failed cleanup below, but at the earlier
    // pre-emplace_back stage where only BeginWidget needs unwinding.
    EndWidget<PopupWindow>();
    tStack.back().mNewSiblings.pop_back();
    return false;
  }
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
  FUI_ASSERT(open);
  if (!*open) {
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
  if (!BeginBasicPopupWindow(id)) {
    return false;
  }

  using namespace StaticTheme::Common;
  static const ImmutableStyle PopupStyles {
    Style()
      .BackgroundColor(AcrylicBackgroundFillColorDefaultBrush)
      .BorderColor(SurfaceStrokeColorDefaultBrush)
      .BorderRadius(OverlayCornerRadius)
      .BorderWidth(2)
      .Padding(20),
  };

  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"PopupWindow/Anchor"}, PopupStyles);

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
