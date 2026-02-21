// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/ScopeableResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp>

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndToolTip();

using ToolTipResult = Result<
  &EndToolTip,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin>;

[[nodiscard]]
ToolTipResult BeginToolTipForPreviousWidget(
  ID id = ID {std::source_location::current()});

namespace immediate_detail {
[[nodiscard]]
ToolTipResult BeginToolTipForWidget(Widgets::Widget*, ID);

}

}// namespace FredEmmott::GUI::Immediate
