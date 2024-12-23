// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "concepts.hpp"

namespace FredEmmott::GUI::Immediate {

template <string_widget T>
struct Leaf {
  template <class... Args>
  void operator()(
    const typename T::Options& options,
    std::format_string<Args...> fmt,
    Args&&... args) const {
    using namespace immediate_detail;

    const auto [id, text] = ParsedID::Make<T>(fmt, std::forward<Args>(args)...);

    TruncateUnlessNextIdEquals(id);

    auto& frame = tStack.back();
    auto& [siblings, i] = frame;
    if (i == siblings.size()) {
      siblings.push_back(new T(id, options));
    }

    string_widget auto* child = static_cast<T*>(siblings.at(i));
    if (child->GetText() != text) {
      child->SetText(text);
    }

    ++i;
  }

  template <class... Args>
  void operator()(std::format_string<Args...> fmt, Args&&... args) const {
    return (*this)({}, fmt, std::forward<Args>(args)...);
  }
};

}// namespace FredEmmott::GUI::Immediate