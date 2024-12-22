// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/memory.hpp>

#include <yoga/Yoga.h>

namespace FredEmmott::GUI {

using unique_ygnode = ::FredEmmott::Memory::unique_ptr<YGNode, &YGNodeFree>;
using shared_ygnode = ::FredEmmott::Memory::shared_ptr<YGNode, &YGNodeFree>;

}