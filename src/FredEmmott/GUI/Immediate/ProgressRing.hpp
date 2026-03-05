// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ProgressRing.hpp>
#include <FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp>

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {

struct IndeterminateProgressRingResultMixin {
  template <class Self>
  decltype(auto) Active(this Self&& self, const bool active) {
    widget_from_result<Widgets::ProgressRing>(self)->SetIsActive(active);
    return std::forward<Self>(self);
  }
};

}// namespace immediate_detail

inline Result<
  nullptr,
  void,
  immediate_detail::IndeterminateProgressRingResultMixin,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin>
ProgressRing(const ID id = ID {std::source_location::current()}) {
  return {
    immediate_detail::ChildlessWidget<Widgets::ProgressRing>(
      id, Widgets::ProgressRing::Kind::Indeterminate),
  };
}

inline Result<
  nullptr,
  void,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin>
ProgressRing(
  const float value,
  const float minimum = 0.0f,
  const float maximum = 100.0f,
  const ID id = ID {std::source_location::current()}) {
  const auto w = immediate_detail::ChildlessWidget<Widgets::ProgressRing>(
    id, Widgets::ProgressRing::Kind::Determinate);
  w->SetRange(minimum, maximum);
  w->SetValue(value);
  return w;
}

}// namespace FredEmmott::GUI::Immediate