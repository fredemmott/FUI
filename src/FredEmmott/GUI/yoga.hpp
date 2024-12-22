// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/Yoga.h>

#include <FredEmmott/memory.hpp>

namespace FredEmmott::Memory::extensions {

template <>
struct deleter<YGNode> {
  void operator()(YGNode* p) const {
    YGNodeFree(p);
  }
};

}// namespace FredEmmott::Memory::extensions