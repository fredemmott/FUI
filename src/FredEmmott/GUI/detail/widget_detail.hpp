// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <vector>

namespace FredEmmott::GUI::Widgets::widget_detail {

void AssignChildren(
  std::vector<unique_ptr<Widget>>* store,
  const std::vector<Widget*>& newChildren);

}