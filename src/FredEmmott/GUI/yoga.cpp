// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "yoga.hpp"

#include <mutex>

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

Size GetMinimumWidthAndIdealHeight(YGNodeConstRef original) {
  // We clone the node due to caching bugs with YGNodeCalculateLayout with
  // varying sizes, and instead set the width property on the clone.
  //
  // This workaround is suggested here:
  //
  // https://github.com/facebook/yoga/issues/1003#issuecomment-642888983
  const unique_ptr<YGNode> owned {YGNodeClone(original)};
  const auto yoga = owned.get();
  YGNodeStyleSetOverflow(yoga, YGOverflowVisible);
  YGNodeStyleSetFlexDirection(yoga, YGFlexDirectionRow);
  YGNodeStyleSetWidth(yoga, YGUndefined);
  YGNodeStyleSetHeight(yoga, YGUndefined);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  float low = 128;
  float high = YGNodeLayoutGetWidth(yoga);
  while ((high - low) > 1) {
    const auto mid = std::ceil((low + high) / 2);
    YGNodeStyleSetWidth(yoga, mid);
    YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
    if (YGNodeLayoutGetHadOverflow(yoga)) {
      if (mid - low < 1) {
        break;
      }
      low = mid;
    } else {
      if (high - mid < 1) {
        break;
      }
      high = mid;
    }
  }
  YGNodeStyleSetWidth(yoga, high);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  return Size {
    YGNodeLayoutGetWidth(yoga),
    YGNodeLayoutGetHeight(yoga),
  };
}
}// namespace FredEmmott::GUI