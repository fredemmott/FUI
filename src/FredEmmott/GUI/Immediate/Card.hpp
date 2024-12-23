// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Card.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

namespace FredEmmott::GUI::Immediate {
using CardOptions = Widgets::Card::Options;

void BeginCard(const CardOptions& options = {});
void BeginCard(const CardOptions& options, immediate_detail::MangledID id);
void BeginCard(const CardOptions& options, auto id) {
  using namespace immediate_detail;
  return BeginCard(options, MangledID {MakeID<Widgets::Card>(id)});
}
void EndCard();

}// namespace FredEmmott::GUI::Immediate