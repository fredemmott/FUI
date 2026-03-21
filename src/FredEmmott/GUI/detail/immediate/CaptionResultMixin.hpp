// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <FredEmmott/GUI/Immediate/ID.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <utility>

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct CaptionResultMixin {
  template <class Self>
  decltype(auto) Caption(this Self&& self, const std::string_view label) {
    AttachCaption(widget_from_result(self), label);
    return std::forward<Self>(self);
  }

  template <class Self, class... Args>
    requires(sizeof...(Args) >= 1)
  decltype(auto)
  Caption(this Self&& self, std::format_string<Args...> fmt, Args&&... args) {
    const auto [id, text] = ParsedID(fmt, std::forward<Args>(args)...);
    AttachCaption(widget_from_result(self), text, id);
    return std::forward<Self>(self);
  }

 private:
  static void AttachCaption(Widget* w, std::string_view label);
  static void AttachCaption(Widget* w, std::string_view label, const ID&);
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail