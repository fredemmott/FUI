// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "concepts.hpp"

namespace FredEmmott::GUI::Immediate::SingleChildWidget {

template <single_child_widget T>
struct Begin {
  using Options = typename T::Options;

  void operator()(const Options& options = {}) const {
    return (*this)(options, immediate_detail::MakeID<T>());
  }

  void operator()(const Options& options, immediate_detail::MangledID id)
    const {
    using namespace immediate_detail;
    TruncateUnlessNextIdEquals(id);

    auto& [siblings, i] = tStack.back();
    if (i == siblings.size()) {
      siblings.push_back(new T(id, options));
    }

    const auto child = static_cast<T*>(siblings.at(i))->GetChild();
    if (child) {
      tStack.emplace_back(std::vector {child});
    } else {
      tStack.emplace_back();
    }
  }

  void operator()(const Options& options, auto id) const
    requires(!std::convertible_to<immediate_detail::MangledID, decltype(id)>)
  {
    return (*this)(options, immediate_detail::MakeID<T>(id));
  }
};

template <single_child_widget T>
struct End {
  void operator()() const {
    using namespace immediate_detail;

    const auto back = std::move(tStack.back());
    if (back.mChildren.size() > 1) [[unlikely]] {
      throw std::logic_error("Only one child is supported");
    }
    tStack.pop_back();

    auto& [siblings, i] = tStack.back();
    const auto widget = static_cast<T*>(siblings.at(i));
    if (back.mChildren.empty()) {
      widget->SetChild(nullptr);
    } else {
      widget->SetChild(back.mChildren.back());
    }
    ++i;
  }
};

}// namespace FredEmmott::GUI::Immediate::SingleChildWidget