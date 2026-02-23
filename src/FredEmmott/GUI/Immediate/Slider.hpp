// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Slider.hpp>
#include <FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {
struct SliderResultMixin {
  using value_formatter_t = std::string (*)(float);
  template <class Self>
  decltype(auto) Range(this Self&& self, float min, float max) {
    widget_from_result<Widgets::Slider>(self)->SetRange(min, max);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) StepFrequency(this Self&& self, float frequency) {
    widget_from_result<Widgets::Slider>(self)->SetStepFrequency(frequency);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) TickFrequency(this Self&& self, float frequency) {
    widget_from_result<Widgets::Slider>(self)->SetTickFrequency(frequency);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) SnapToTicks(this Self&& self) {
    widget_from_result<Widgets::Slider>(self)->SetSnapTo(
      Widgets::Slider::SnapTo::Ticks);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) ValueFormatter(
    this Self&& self,
    const value_formatter_t formatter) {
    SetValueFormatter(widget_from_result<Widgets::Slider>(self), formatter);
    return std::forward<Self>(self);
  }

 private:
  static void SetValueFormatter(Widgets::Slider*, value_formatter_t);
};
}// namespace immediate_detail

using SliderResult = Result<
  nullptr,
  bool,
  immediate_detail::CaptionResultMixin,
  immediate_detail::SliderResultMixin,
  immediate_detail::ToolTipResultMixin>;

/** Create a horizontal slider.
 *
 * If the value is changed by user interaction, *pValue is updated and the
 * result is truthy.
 */
SliderResult HSlider(
  float* pValue,
  ID id = ID {std::source_location::current()});

/** Create a vertical slider.
 *
 * If the value is changed by user interaction, *pValue is updated and the
 * result is truthy.
 */
SliderResult VSlider(
  float* pValue,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate