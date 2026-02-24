// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <felly/numeric_cast.hpp>

#include "FredEmmott/GUI/Immediate/Result.hpp"
#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {
struct NumberBoxResultMixin {
  using value_formatter_t = std::string (*)(float);
  using value_filter_t = float (*)(float oldValue, float newValue);

  template <class Self>
  decltype(auto) Range(this Self&& self, const float min, const float max) {
    SetRange(widget_from_result<Widgets::TextBox>(self), min, max);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) SmallStep(this Self&& self, float interval) {
    SetSmallStep(widget_from_result<Widgets::TextBox>(self), interval);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) ValueFormatter(
    this Self&& self,
    const value_formatter_t formatter) {
    SetValueFormatter(widget_from_result<Widgets::TextBox>(self), formatter);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) ValueFilter(this Self&& self, const value_filter_t filter) {
    SetValueFilter(widget_from_result<Widgets::TextBox>(self), filter);
    return std::forward<Self>(self);
  }

 private:
  static void SetRange(Widgets::Widget* w, float min, float max);
  static void SetSmallStep(Widgets::Widget* w, float interval);
  static void SetValueFormatter(Widgets::Widget*, value_formatter_t);
  static void SetValueFilter(Widgets::Widget*, value_filter_t);
};

}// namespace immediate_detail

using NumberBoxResult = Result<
  &immediate_detail::EndWidget<Widgets::TextBox>,
  bool,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin,
  immediate_detail::NumberBoxResultMixin>;

NumberBoxResult NumberBox(
  float* value,
  ID id = ID {std::source_location::current()});

template <std::integral T>
NumberBoxResult NumberBox(
  std::optional<T>* optValue,
  const ID id = ID {std::source_location::current()}) {
  if (!optValue) [[unlikely]] {
    throw std::logic_error("Must have a pointer for value");
  }
  float f = optValue->has_value()
    ? felly::numeric_cast<float>(optValue->value())
    : std::numeric_limits<float>::quiet_NaN();

  const auto ret = NumberBox(&f, id);

  if (ret) {
    if (std::isnan(f)) {
      *optValue = std::nullopt;
    } else {
      *optValue = felly::numeric_cast<T>(std::llround(f));
    }
  }

  return ret;
}

}// namespace FredEmmott::GUI::Immediate
