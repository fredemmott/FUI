// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "concepts.hpp"

namespace FredEmmott::GUI::Immediate {

template <string_widget T>
struct Leaf {
  template <class... Args>
  void operator()(std::format_string<Args...> fmt, Args&&... args) const {
    using namespace immediate_detail;

    const auto [id, text] = ParsedID::Make<T>(fmt, std::forward<Args>(args)...);
    auto& [siblings, i] = tStack.back();
    if (i < siblings.size()) {
      if (siblings.at(i)->GetID() != id) {
        siblings.erase(siblings.begin() + i, siblings.end());
      }
    }

    if (i >= siblings.size()) {
      siblings.push_back(new T(id, {}));
    }

    string_widget auto* child = static_cast<T*>(siblings.back());
    if (child->GetText() != text) {
      child->SetText(text);
    }

    ++i;
  }
};

}// namespace FredEmmott::GUI::Immediate