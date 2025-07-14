// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ResizeToFit.hpp"

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

namespace FredEmmott::GUI::Immediate {

void ResizeToFit() {
  immediate_detail::tResizeNextFrame = true;
  immediate_detail::tNeedAdditionalFrame = true;
}

}// namespace FredEmmott::GUI::Immediate
