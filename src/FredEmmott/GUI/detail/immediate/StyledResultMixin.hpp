// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "WidgetlessResultMixin.hpp"
#include "widget_from_result.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {
template <class... TMixins>
struct StyledResultMixin {};

template <class... TMixins>
  requires(!(std::same_as<TMixins, WidgetlessResultMixin> || ...))
struct StyledResultMixin<TMixins...> {
  template <class Self>
  decltype(auto) Styled(this Self&& self, const Style& style) {
    widget_from_result(self)->SetMutableStyles(style);
    return std::forward<Self>(self);
  }
};
}// namespace FredEmmott::GUI::Immediate::immediate_detail