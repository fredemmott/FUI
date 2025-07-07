// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>
#include <FredEmmott/GUI/Immediate/ID.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <format>
#include <span>

#include "widget_detail.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

using Widget = Widgets::Widget;
using namespace Widgets::widget_detail;

struct StackEntry final {
  std::vector<Widget*> mPending;
  std::vector<Widget*> mNewSiblings;
  uint64_t mNextIndex {};
};

extern thread_local std::vector<StackEntry> tStack;
extern thread_local Window* tWindow;
extern thread_local ActivatedFlag tNeedAdditionalFrame;
extern thread_local ActivatedFlag tResizeToFit;

template <std::derived_from<Widget> T = Widget>
T* GetCurrentNode() {
  if (tStack.back().mNewSiblings.empty()) {
    return nullptr;
  }
  return widget_cast<T>(tStack.back().mNewSiblings.back());
}

template <std::derived_from<Widget> T = Widget>
T* GetCurrentParentNode() {
  if (tStack.empty()) {
    return nullptr;
  }
  const auto& frame = tStack.at(tStack.size() - 2);
  return widget_cast<T>(frame.mNewSiblings.back());
}

struct ParsedID {
  ID mID {0};
  std::string mText;

  ParsedID() = delete;
  template <class... Args>
  explicit ParsedID(std::format_string<Args...> fmt, Args&&... args) {
    const auto formatted = std::format(fmt, std::forward<Args>(args)...);

    const auto fmtView = fmt.get();
    const auto i = fmtView.rfind("##");
    if (i == std::string_view::npos) {
      mID = ID {formatted};
      mText = formatted;
      return;
    }

    const auto j = formatted.rfind("##");
    mID = ID {formatted.substr(j + 2)};
    mText = formatted.substr(0, j);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail