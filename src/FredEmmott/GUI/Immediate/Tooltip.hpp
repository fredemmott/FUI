// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/ScopeableResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp>

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndTooltip();

using TooltipResult = Result<
  &EndTooltip,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin>;

[[nodiscard]]
TooltipResult BeginTooltipForPreviousWidget(
  const ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate
