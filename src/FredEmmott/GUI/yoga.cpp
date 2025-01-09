// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <thread>

#include "yoga.hpp"

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
}