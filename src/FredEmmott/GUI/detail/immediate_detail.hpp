// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <format>
#include <functional>
#include <span>

namespace FredEmmott::GUI::Immediate::immediate_detail {

using namespace FredEmmott::GUI::Widgets;

template <std::derived_from<Widget> T>
T* widget_cast(Widget* const p) {
  return dynamic_cast<T*>(p);
}

struct StackEntry final {
  std::vector<Widget*> mChildren;
  uint64_t mNextIndex {};
};

extern thread_local std::vector<StackEntry> tStack;

template <std::derived_from<Widget> T = Widget>
T* GetCurrentNode() {
  const auto& [nodes, i] = tStack.back();
  if (i == nodes.size()) {
    return nullptr;
  }
  return widget_cast<T>(nodes.at(i));
}

void TruncateUnlessNextIdEquals(std::size_t id);

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