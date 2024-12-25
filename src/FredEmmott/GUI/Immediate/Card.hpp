// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Card.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

constexpr immediate_detail::BeginWidget<Widgets::Card> BeginCard;

inline void EndCard() {
  immediate_detail::EndWidget<Widgets::Card>();
}

}// namespace FredEmmott::GUI::Immediate