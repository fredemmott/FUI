// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Lerp.hpp"

#include "Brush.hpp"
#include "StyleTransitions.hpp"

namespace FredEmmott::GUI {

Brush Lerp(const Brush& startRef, const Brush& endRef, double ratio) {
  const auto start = startRef.GetSolidColor();
  const auto end = endRef.GetSolidColor();
  if (!(start && end)) {
    return ratio < 0.5 ? startRef : endRef;
  }
  return Lerp(*start, *end, ratio);
}

}// namespace FredEmmott::GUI