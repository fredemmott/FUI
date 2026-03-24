// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/Yoga.h>

#include <felly/scope_exit.hpp>
#include <felly/unique_ptr.hpp>

#include "Size.hpp"
#include "assert.hpp"

struct YGConfig;
struct YGNode;
struct YGSize;

namespace FredEmmott::GUI {

using unique_yoga_node_ptr = felly::unique_ptr<YGNode, &YGNodeFree>;
using unique_yoga_config_ptr = felly::unique_ptr<YGConfig, &YGConfigFree>;

YGConfig* GetYogaConfig();
float GetMinimumWidth(const YGNode* node);
float GetMinimumWidth(const YGNode* node, float hint);
enum class ClampedMinimumWidthHint {
  None,
  MinimumIsLikely,
};

float GetClampedMinimumWidth(
  const YGNode* node,
  float min,
  float max,
  ClampedMinimumWidthHint hint = ClampedMinimumWidthHint::None);

float GetIdealHeight(const YGNode* node, float width);
Size GetMinimumWidthAndIdealHeight(const YGNode* node);

enum class CSSMeasureMode : std::underlying_type_t<YGMeasureMode> {
  StretchFit = YGMeasureModeExactly,
  MaxContent = YGMeasureModeUndefined,
  FitContent = YGMeasureModeAtMost,
};

[[nodiscard]]
#ifdef JETBRAINS_IDE
[[jetbrains::guard]]
#endif
inline auto ScopedYogaParentCheck([[maybe_unused]] const YGNode* const node) {
#ifdef NDEBUG
  struct dummy {};
  return dummy {};
#else
  const auto childCount = YGNodeGetChildCount(node);
  if (childCount > 0) {
    FUI_ALWAYS_ASSERT(
      YGNodeGetParent(YGNodeGetChild(const_cast<YGNode*>(node), 0)) == node);
  }
  return felly::scope_exit([node, childCount] {
    FUI_ALWAYS_ASSERT(childCount == YGNodeGetChildCount(node));
    if (childCount > 0) {
      FUI_ALWAYS_ASSERT(
        YGNodeGetParent(YGNodeGetChild(const_cast<YGNode*>(node), 0)) == node,
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
inline void FixYogaChildren(YGNode* const parent) {
  const auto childCount = YGNodeGetChildCount(parent);
  for (auto i = 0; i < childCount; ++i) {
    YGNodeSwapChild(parent, YGNodeGetChild(parent, i), i);
  }
}

}// namespace FredEmmott::GUI