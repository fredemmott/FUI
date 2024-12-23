// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "concepts.hpp"

namespace FredEmmott::GUI::Immediate::SingleChildWidget {

template <single_child_widget T>
struct Begin {
  void operator()(const WidgetStyles& styles = {}) const {
    return (*this)(styles, immediate_detail::MakeID<T>());
  }

  void operator()(const WidgetStyles& styles, immediate_detail::MangledID id)
    const {
    using namespace immediate_detail;
    TruncateUnlessNextIdEquals(id);

    auto& [siblings, i] = tStack.back();
    if (i == siblings.size()) {
      siblings.push_back(new T(id));
    }

    auto it = GetCurrentNode<T>();
    it->SetExplicitStyles(styles);

    if (const auto child = it->GetChild()) {
      tStack.emplace_back(std::vector {child});
    } else {
      tStack.emplace_back();
    }
  }

  void operator()(const WidgetStyles& styles, auto id) const
    requires(!std::convertible_to<immediate_detail::MangledID, decltype(id)>)
  {
    return (*this)(styles, immediate_detail::MakeID<T>(id));
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