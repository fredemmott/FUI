// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <felly/scope_exit.hpp>
#include <felly/unique_ptr.hpp>

#include "Size.hpp"
#include "assert.hpp"

struct YGConfig;
struct YGNode;
struct YGSize;
extern "C" void YGNodeFree(YGNode*);
extern "C" void YGConfigFree(YGConfig*);

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

struct [[nodiscard]]
#ifdef JETBRAINS_IDE
[[jetbrains::guard]]
#endif
ScopedYogaParentCheck final {
  ScopedYogaParentCheck() = delete;
  explicit ScopedYogaParentCheck(const YGNode*);
  ~ScopedYogaParentCheck();

  ScopedYogaParentCheck(const ScopedYogaParentCheck&) = delete;
  ScopedYogaParentCheck(ScopedYogaParentCheck&&) = delete;
  ScopedYogaParentCheck& operator=(const ScopedYogaParentCheck&) = delete;
  ScopedYogaParentCheck& operator=(ScopedYogaParentCheck&&) = delete;

 private:
  struct dummy_t {};
  struct input_t {
    const YGNode* mNode;
    size_t mChildCount {};
  };
  FELLY_NO_UNIQUE_ADDRESS
  std::conditional_t<Config::Debug, input_t, dummy_t> mInput {};
};

/** Working on a cloned node can break parent-child relationships; fix them.
 *
 * This appears expected behavior, e.g. similar workaround in React Native in
 * https://github.com/facebook/react-native/blob/47c7165c3a3ba0ebe7e67a857bebacb82bbdb2c6/packages/react-native/React/Views/RCTShadowView.m#L395
 */
void FixYogaChildren(YGNode* parent);

}// namespace FredEmmott::GUI