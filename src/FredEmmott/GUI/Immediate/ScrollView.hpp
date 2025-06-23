// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

inline void BeginVScrollView(
  const ID id = ID {std::source_location::current()}) {
  using Widgets::ScrollView;
  using namespace immediate_detail;
  BeginWidget<ScrollView>(id);
  GetCurrentParentNode<ScrollView>()->SetHorizontalScrollBarVisibility(
    ScrollView::ScrollBarVisibility::Hidden);
  GetCurrentParentNode<ScrollView>()->SetVerticalScrollBarVisibility(
    ScrollView::ScrollBarVisibility::Auto);
}

inline void EndVScrollView() {
  immediate_detail::EndWidget<Widgets::ScrollView>();
}

}// namespace FredEmmott::GUI::Immediate
