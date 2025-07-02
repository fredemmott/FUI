// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndVScrollView() {
  immediate_detail::EndWidget<Widgets::ScrollView>();
}

inline Result<&EndVScrollView> BeginVScrollView(
  const ID id = ID {std::source_location::current()}) {
  using Widgets::ScrollView;
  using namespace immediate_detail;
  const auto ret = BeginWidget<ScrollView>(id);
  ret->SetHorizontalScrollBarVisibility(
    ScrollView::ScrollBarVisibility::Hidden);
  ret->SetVerticalScrollBarVisibility(ScrollView::ScrollBarVisibility::Auto);
  return {ret};
}

}// namespace FredEmmott::GUI::Immediate
