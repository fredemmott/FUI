// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/Yoga.h>

#include <FredEmmott/memory.hpp>

#include "Size.hpp"

namespace FredEmmott::Memory::extensions {
template <>
struct deleter<YGNode> {
  void operator()(YGNode* p) const {
    YGNodeFree(p);
  }
};

template <>
struct deleter<YGConfig> {
  void operator()(YGConfig* p) const {
    YGConfigFree(p);
  }
};
}// namespace FredEmmott::Memory::extensions

namespace FredEmmott::GUI {
using namespace FredEmmott::Memory;

YGConfigRef GetYogaConfig();
float GetMinimumWidth(YGNodeConstRef node);
float GetMinimumWidth(YGNodeConstRef node, float hint);
enum class ClampedMinimumWidthHint {
  None,
  MinimumIsLikely,
};

float GetClampedMinimumWidth(
  YGNodeConstRef node,
  float min,
  float max,
  ClampedMinimumWidthHint hint = ClampedMinimumWidthHint::None);

float GetIdealHeight(YGNodeConstRef node, float width);
Size GetMinimumWidthAndIdealHeight(YGNodeConstRef node);

enum class CSSMeasureMode : std::underlying_type_t<YGMeasureMode> {
  StretchFit = YGMeasureModeExactly,
  MaxContent = YGMeasureModeUndefined,
  FitContent = YGMeasureModeAtMost,
};

}// namespace FredEmmott::GUI