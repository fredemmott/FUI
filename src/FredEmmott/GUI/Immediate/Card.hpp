// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Card.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "SingleChildWidget.hpp"

namespace FredEmmott::GUI::Immediate {
constexpr SingleChildWidget::Begin<Widgets::Card> BeginCard;
constexpr SingleChildWidget::End<Widgets::Card> EndCard;

}// namespace FredEmmott::GUI::Immediate