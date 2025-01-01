// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Card.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

inline void BeginCard(ID id = ID {std::source_location::current()}) {
  immediate_detail::BeginWidget<Widgets::Card>(ID {id});
}

inline void EndCard() {
  immediate_detail::EndWidget<Widgets::Card>();
}

}// namespace FredEmmott::GUI::Immediate