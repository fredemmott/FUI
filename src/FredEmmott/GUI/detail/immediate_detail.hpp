// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <format>
#include <functional>
#include <span>

#include "widget_detail.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

using namespace Widgets;
using namespace widget_detail;

struct StackEntry final {
  std::vector<Widget*> mPending;
  std::vector<Widget*> mNewSiblings;
  uint64_t mNextIndex {};
};

extern thread_local std::vector<StackEntry> tStack;

template <std::derived_from<Widget> T = Widget>
T* GetCurrentNode() {
  return widget_cast<T>(tStack.back().mNewSiblings.back());
}

template <std::derived_from<Widget> T = Widget>
T* GetPreviousNode() {
  const auto& frame = tStack.back();
  return widget_cast<T>(frame.mNewSiblings.end() - 2);
}

template <std::derived_from<Widget> T = Widget>
T* GetCurrentParentNode() {
  if (tStack.size() < 1) {
    return nullptr;
  }
  const auto& frame = tStack.at(tStack.size() - 2);
  return widget_cast<T>(frame.mNewSiblings.back());
}

class MangledID {
 public:
  MangledID() = delete;
  constexpr explicit MangledID(std::size_t id) : mValue(id) {
  }

  constexpr operator std::size_t() const noexcept {
    return mValue;
  }

  std::size_t mValue;
};

template <class T>
auto MakeID(auto data) {
  return MangledID {typeid(T).hash_code() ^ std::hash<decltype(data)> {}(data)};
}
template <class T>
auto MakeID() {
  return MakeID<T>(tStack.back().mNextIndex);
}

struct ParsedID {
  MangledID mID;
  std::string mText;

  template <class T, class... Args>
  static ParsedID Make(std::format_string<Args...> fmt, Args&&... args) {
    static constexpr std::hash<std::string_view> Hash;

    const auto formatted = std::format(fmt, std::forward<Args>(args)...);

    ParsedID ret {
      .mID = MangledID {typeid(T).hash_code()},
    };

    const auto fmtView = fmt.get();
    const auto i = fmtView.rfind("##");
    if (i == std::string_view::npos) {
      ret.mText = formatted;
      ret.mID.mValue ^= Hash(formatted);
      return ret;
    }

    const auto j = formatted.rfind("##");
    ret.mText = formatted.substr(0, j);
    ret.mID.mValue ^= Hash(formatted.substr(j + 2));
    return ret;
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail