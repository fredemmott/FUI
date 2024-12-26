// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StyleTransitions.hpp"

#include "Brush.hpp"
#include "interpolate.hpp"

namespace FredEmmott::GUI {

Brush interpolate(const Brush& startRef, const Brush& endRef, double ratio) {
  const auto start = startRef.GetSolidColor();
  const auto end = endRef.GetSolidColor();
  if (!(start && end)) {
    return ratio < 0.5 ? startRef : endRef;
  }
  return interpolate(*start, *end, ratio);
}

}// namespace FredEmmott::GUI