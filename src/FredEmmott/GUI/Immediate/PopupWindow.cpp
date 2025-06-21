// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include <FredEmmott/GUI/Widgets/PopupWindow.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

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

bool BeginPopupWindow(const ID id) {
  auto previous = GetCurrentNode();

  BeginWidget<PopupWindow>(id);
  auto window = GetCurrentParentNode<PopupWindow>()->GetWindow();
  window->SetParent(tWindow->GetNativeHandle());
  if (previous && !window->GetNativeHandle()) {
    SkIPoint position {};
    for (auto node = previous->GetLayoutNode(); node;) {
      position.fX += YGNodeLayoutGetLeft(node);
      position.fY += YGNodeLayoutGetTop(node);
      auto ctx = static_cast<YogaContext*>(YGNodeGetContext(node));
      const auto tree = std::get_if<DetachedYogaTree>(ctx);
      if (tree) {
        node = tree->mRealParent;
        position.fX -= tree->mOffset.fX;
        position.fY -= tree->mOffset.fY;
      } else {
        node = YGNodeGetParent(node);
      }
    }
    position = tWindow->CanvasPointToNativePoint(position);
    window->SetInitialPosition(position);
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

bool BeginPopupWindow(bool* open, ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginPopupWindow(id);
  return *open;
}

void EndPopupWindow() {
  auto window = tPopupStack.back().mWindow;
  window->EndFrame();
  PopParentContext();
  EndWidget<PopupWindow>();
}

}// namespace FredEmmott::GUI::Immediate