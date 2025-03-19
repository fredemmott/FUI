// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Linear.hpp"

#include "FredEmmott/GUI/Font.hpp"

namespace FredEmmott::GUI::Interpolation {

Brush Linear(const Brush& startRef, const Brush& endRef, float ratio) {
  const auto start = startRef.GetSolidColor();
  const auto end = endRef.GetSolidColor();
  if (!(start && end)) {
    return ratio < 0.5 ? startRef : endRef;
  }
  return Linear(*start, *end, ratio);
}

Font Linear(const Font& startRef, const Font& endRef, float ratio) {
  const auto start = startRef.GetFontSizeInPixels();
  const auto end = startRef.GetFontSizeInPixels();
  if (std::abs(start - end) < std::numeric_limits<float>::epsilon() * start) {
    return endRef;
  }
  return endRef.WithSizeInPixels(Linear(start, end, ratio));
}

}// namespace FredEmmott::GUI::Interpolation