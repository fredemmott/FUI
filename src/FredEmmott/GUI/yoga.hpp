// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/Yoga.h>

#include <FredEmmott/memory.hpp>
#include <felly/scope_exit.hpp>

#include "Size.hpp"
#include "assert.hpp"

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

[[nodiscard]]
#ifdef JETBRAINS_IDE
[[jetbrains::guard]]
#endif
inline auto ScopedYogaParentCheck([[maybe_unused]] YGNodeConstRef const node) {
#ifdef NDEBUG
  struct dummy {};
  return dummy {};
#else
  const auto childCount = YGNodeGetChildCount(node);
  if (childCount > 0) {
    FUI_ALWAYS_ASSERT(
      YGNodeGetParent(YGNodeGetChild(const_cast<YGNodeRef>(node), 0)) == node);
  }
  return felly::scope_exit([node, childCount] {
    FUI_ALWAYS_ASSERT(childCount == YGNodeGetChildCount(node));
    if (childCount > 0) {
      FUI_ALWAYS_ASSERT(
        YGNodeGetParent(YGNodeGetChild(const_cast<YGNodeRef>(node), 0)) == node,
        "Children have incorrect parent; consider using `FixYogaChildren()`");
    }
  });
#endif
}

/** Working on a cloned node can break parent-child relationships; fix them.
 *
 * This appears expected behavior, e.g. similar workaround in React Native in
 * https://github.com/facebook/react-native/blob/47c7165c3a3ba0ebe7e67a857bebacb82bbdb2c6/packages/react-native/React/Views/RCTShadowView.m#L395
 */
inline void FixYogaChildren(YGNodeRef const parent) {
  const auto childCount = YGNodeGetChildCount(parent);
  for (auto i = 0; i < childCount; ++i) {
    YGNodeSwapChild(parent, YGNodeGetChild(parent, i), i);
  }
}

}// namespace FredEmmott::GUI