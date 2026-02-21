// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Label.hpp>
#include <FredEmmott/GUI/Immediate/ToolTip.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct ToolTipResultMixin {
  template <class Self>
  decltype(auto) ToolTip(
    this Self&& self,
    const std::string_view toolTip,
    const ID id = ID {std::source_location::current()}) {
    if (
      const auto tt
      = BeginToolTipForWidget(widget_from_result(self), id).Scoped()) {
      Label(toolTip);
    }
    return std::forward<Self>(self);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail
