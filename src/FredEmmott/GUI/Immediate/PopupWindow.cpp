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
  Window* mWindow {nullptr};
  decltype(tStack) mWindowStack;
};
thread_local std::vector<ParentContext> tPopupStack;

void PopParentContext() {
  tStack = std::move(tPopupStack.back().mWindowStack);
  tPopupStack.pop_back();
}

}// namespace

bool BeginPopupWindow(const ID id) {
  BeginWidget<PopupWindow>(id);
  auto window = GetCurrentParentNode<PopupWindow>()->GetWindow();

  tPopupStack.emplace_back(window, std::move(tStack));
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
  const auto shown = BeginPopupWindow(id);
  return *open = shown;
}

void EndPopupWindow() {
  auto window = tPopupStack.back().mWindow;
  window->EndFrame();
  PopParentContext();
  EndWidget<PopupWindow>();
}

}// namespace FredEmmott::GUI::Immediate