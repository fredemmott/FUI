// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Card.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

inline void EndCard() {
  immediate_detail::EndWidget<Widgets::Card>();
}

inline Result<&EndCard> BeginCard(
  ID id = ID {std::source_location::current()}) {
  return {immediate_detail::BeginWidget<Widgets::Card>(ID {id})};
}

}// namespace FredEmmott::GUI::Immediate