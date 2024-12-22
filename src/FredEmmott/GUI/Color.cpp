// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Color.hpp"

namespace FredEmmott::GUI {

Color Color::MixIn(SkScalar rhsRatio, const Color& rhs) const noexcept {
  const auto lhsRatio = 1 - rhsRatio;
  const auto& lhs = *this;
  return SkColorSetARGB(
    (SkColorGetA(lhs) * lhsRatio) + (SkColorGetA(rhs) * rhsRatio),
    (SkColorGetR(lhs) * lhsRatio) + (SkColorGetR(rhs) * rhsRatio),
    (SkColorGetG(lhs) * lhsRatio) + (SkColorGetG(rhs) * rhsRatio),
    (SkColorGetB(lhs) * lhsRatio) + (SkColorGetB(rhs) * rhsRatio));

}
}// namespace FredEmmott::GUI