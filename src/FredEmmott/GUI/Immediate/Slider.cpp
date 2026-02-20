// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Slider.hpp"

#include <stdexcept>
#include <utility>

namespace FredEmmott::GUI::Immediate {

namespace {
[[nodiscard]]
SliderResult
SliderImpl(float* const pValue, const Orientation orientation, const ID id) {
  if (!pValue) [[unlikely]] {
    throw std::logic_error("Slider requires a non-null value pointer");
  }

  const auto w
    = immediate_detail::ChildlessWidget<Widgets::Slider>(id, orientation);

  const auto changed = std::exchange(w->mChanged, false);
  if (changed) {
    *pValue = w->GetValue();
  } else {
    w->SetValue(*pValue);
  }

  return {w, changed};
}
}// namespace

SliderResult HSlider(float* pValue, const ID id) {
  return SliderImpl(pValue, Orientation::Horizontal, id);
}

SliderResult VSlider(float* pValue, const ID id) {
  return SliderImpl(pValue, Orientation::Vertical, id);
}

}// namespace FredEmmott::GUI::Immediate