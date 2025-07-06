// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndContentDialog();
using ContentDialogResult = Result<
  &EndContentDialog,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopedResultMixin>;

}// namespace FredEmmott::GUI::Immediate