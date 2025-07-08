// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "yoga.hpp"

#include <mutex>
#include <optional>

#include "assert.hpp"

namespace FredEmmott::GUI {
YGConfigRef GetYogaConfig() {
  static unique_ptr<YGConfig> sInstance;
  static std::once_flag sOnceFlag;
  std::call_once(sOnceFlag, [&ret = sInstance]() {
    ret.reset(YGConfigNew());
    YGConfigSetUseWebDefaults(ret.get(), true);
    YGConfigSetPointScaleFactor(ret.get(), 0.0f);
  });
  return sInstance.get();
}

float GetMinimumWidth(YGNodeConstRef node, float hint) {
  // We clone the node due to caching bugs with YGNodeCalculateLayout with
  // varying sizes, and instead set the width property on the clone.
  //
  // This workaround is suggested here:
  //
  // https://github.com/facebook/yoga/issues/1003#issuecomment-642888983
  unique_ptr<YGNode> owned {YGNodeClone(node)};
  auto yoga = owned.get();
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
  return GetClampedMinimumWidth(node, 128, YGNodeLayoutGetWidth(yoga));
}

float GetClampedMinimumWidth(
  YGNodeConstRef node,
  float min,
  float max,
  ClampedMinimumWidthHint hint) {
  unique_ptr<YGNode> owned {YGNodeClone(node)};
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
float GetMinimumWidth(YGNodeConstRef node) {
  return GetMinimumWidth(node, YGUndefined);
}

float GetIdealHeight(YGNodeConstRef node, float width) {
  const unique_ptr<YGNode> owned {YGNodeClone(node)};
  const auto yoga = owned.get();
  YGNodeStyleSetOverflow(yoga, YGOverflowVisible);
  YGNodeStyleSetFlexDirection(yoga, YGFlexDirectionRow);
  YGNodeStyleSetWidth(yoga, width);
  YGNodeStyleSetHeight(yoga, YGUndefined);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  return YGNodeLayoutGetHeight(yoga);
}

Size GetMinimumWidthAndIdealHeight(YGNodeConstRef original) {
  const auto width = GetMinimumWidth(original);
  return Size {
    width,
    GetIdealHeight(original, width),
  };
}
}// namespace FredEmmott::GUI