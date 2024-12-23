// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <functional>
#include <span>

namespace FredEmmott::GUI::Immediate::immediate_detail {

using namespace FredEmmott::GUI::Widgets;

struct StackEntry final {
  std::vector<Widget*> mChildren;
  uint64_t mNextIndex {};
};

extern thread_local std::vector<StackEntry> tStack;

void TruncateUnlessNextIdEquals(std::size_t id);

class MangledID {
 public:
  MangledID() = delete;
  constexpr explicit MangledID(std::size_t id) : mID(id) {
  }

  constexpr operator std::size_t() const noexcept {
    return mID;
  }

 private:
  std::size_t mID;
};

template <class T>
std::size_t MakeID(auto data) {
  return typeid(T).hash_code() ^ std::hash<decltype(data)> {}(data);
}
template <class T>
auto MakeID() {
  return MakeID<T>(tStack.back().mNextIndex);
}

struct ParsedID {
  std::size_t mID;
  std::string mText;

  template <class T, class... Args>
  static ParsedID Make(std::format_string<Args...> fmt, Args&&... args) {
    static constexpr std::hash<std::string_view> Hash;

    const auto formatted = std::format(fmt, std::forward<Args>(args)...);

    ParsedID ret {
      .mID = typeid(T).hash_code(),
    };

    const auto fmtView = fmt.get();
    const auto i = fmtView.rfind("##");
    if (i == std::string_view::npos) {
      ret.mText = formatted;
      ret.mID ^= Hash(formatted);
      return ret;
    }

    const auto j = formatted.rfind("##");
    ret.mText = formatted.substr(0, j);
    ret.mID ^= Hash(formatted.substr(j + 2));
    return ret;
  }

  constexpr operator MangledID() const noexcept {
    return MangledID {mID};
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail