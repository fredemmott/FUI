// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "yoga.hpp"

#include <Yoga.h>

#include <mutex>
#include <optional>

#include "assert.hpp"
#include "detail/win32_detail/UIANode.hpp"

namespace FredEmmott::GUI {
YGConfig* GetYogaConfig() {
  static unique_yoga_config_ptr sInstance {
    [] {
      const auto ret = YGConfigNew();
      YGConfigSetUseWebDefaults(ret, true);
      YGConfigSetPointScaleFactor(ret, 0.0f);
      return ret;
    }(),
  };
  return sInstance.get();
}

float GetMinimumWidth(const YGNode* node, float hint) {
  // We clone the node due to caching bugs with YGNodeCalculateLayout with
  // varying sizes, and instead set the width property on the clone.
  //
  // This workaround is suggested here:
  //
  // https://github.com/facebook/yoga/issues/1003#issuecomment-642888983
  const auto checkParents = ScopedYogaParentCheck(node);
  const auto fixParents
    = felly::scope_exit([node] { FixYogaChildren(const_cast<YGNode*>(node)); });
  const unique_yoga_node_ptr owned {YGNodeClone(node)};
  const auto yoga = owned.get();
  YGNodeStyleSetOverflow(yoga, YGOverflowVisible);
  YGNodeStyleSetFlexDirection(yoga, YGFlexDirectionRow);
  YGNodeStyleSetWidth(yoga, hint);
  YGNodeStyleSetHeight(yoga, YGUndefined);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);

  if (!YGFloatIsUndefined(hint)) {
    if (YGNodeLayoutGetHadOverflow(yoga)) {
      return GetMinimumWidth(node, YGUndefined);
    }
    YGNodeStyleSetWidth(yoga, hint - 2);
    YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
    if (YGNodeLayoutGetHadOverflow(yoga)) {
      return hint;
    }
  }
  const auto contentMax = YGNodeLayoutGetWidth(yoga);

  // SearchMin should be a reasonable value for a normal window, but we can have
  // valid *tiny* windows, like tooltips. If their max size is smaller than
  // SearchMin, we... can't grow it, and the max size is probably the ideal size
  static constexpr auto SearchMin = 128;
  if (contentMax <= SearchMin) {
    return contentMax;
  }
  return GetClampedMinimumWidth(node, 128, contentMax);
}

float GetClampedMinimumWidth(
  const YGNode* node,
  float min,
  float max,
  const ClampedMinimumWidthHint hint) {
  const auto checkParents = ScopedYogaParentCheck(node);
  const auto fixParents
    = felly::scope_exit([node] { FixYogaChildren(const_cast<YGNode*>(node)); });
  const unique_yoga_node_ptr owned {YGNodeClone(node)};
  const auto yoga = owned.get();

  if (hint == ClampedMinimumWidthHint::MinimumIsLikely) {
    YGNodeStyleSetWidth(yoga, min);
    YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
    if (!YGNodeLayoutGetHadOverflow(yoga)) {
      return min;
    }
  }

  while ((max - min) > 1) {
    const auto mid = std::ceil((min + max) / 2);
    YGNodeStyleSetWidth(yoga, mid);
    YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
    if (YGNodeLayoutGetHadOverflow(yoga)) {
      if (mid - min < 1) {
        break;
      }
      min = mid;
    } else {
      if (max - mid < 1) {
        break;
      }
      max = mid;
    }
  }
  return max;
}
float GetMinimumWidth(const YGNode* node) {
  return GetMinimumWidth(node, YGUndefined);
}

float GetIdealHeight(const YGNode* node, float width) {
  const auto checkParents = ScopedYogaParentCheck(node);
  const auto fixParents
    = felly::scope_exit([node] { FixYogaChildren(const_cast<YGNode*>(node)); });

  const unique_yoga_node_ptr owned {YGNodeClone(node)};
  const auto yoga = owned.get();
  YGNodeStyleSetOverflow(yoga, YGOverflowVisible);
  YGNodeStyleSetFlexDirection(yoga, YGFlexDirectionRow);
  YGNodeStyleSetWidth(yoga, width);
  YGNodeStyleSetHeight(yoga, YGUndefined);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  return YGNodeLayoutGetHeight(yoga);
}

Size GetMinimumWidthAndIdealHeight(const YGNode* original) {
  const auto width = GetMinimumWidth(original);
  return Size {
    width,
    GetIdealHeight(original, width),
  };
}
ScopedYogaParentCheck::ScopedYogaParentCheck(
  [[maybe_unused]] const YGNode* const node) {
  if constexpr (Config::Debug) {
    const auto childCount = YGNodeGetChildCount(node);
    if (childCount > 0) {
      FUI_ALWAYS_ASSERT(
        YGNodeGetParent(YGNodeGetChild(const_cast<YGNode*>(node), 0)) == node);
      reinterpret_cast<input_t&>(mInput) = {node, childCount};
    }
  }
}

ScopedYogaParentCheck::~ScopedYogaParentCheck() {
  if constexpr (Config::Debug) {
    const auto [node, childCount] = reinterpret_cast<input_t&>(mInput);
    FUI_ALWAYS_ASSERT(YGNodeGetChildCount(node) == childCount);
    if (childCount > 0) {
      FUI_ALWAYS_ASSERT(
        YGNodeGetParent(YGNodeGetChild(const_cast<YGNode*>(node), 0)) == node,
        "Children have incorrect parent; consider using `FixYogaChildren()`");
    }
  }
}

void FixYogaChildren(YGNode* const parent) {
  const auto childCount = YGNodeGetChildCount(parent);
  for (auto i = 0; i < childCount; ++i) {
    YGNodeSwapChild(parent, YGNodeGetChild(parent, i), i);
  }
}

}// namespace FredEmmott::GUI