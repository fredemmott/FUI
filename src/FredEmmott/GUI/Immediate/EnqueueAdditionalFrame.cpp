// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "EnqueueAdditionalFrame.hpp"

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

namespace FredEmmott::GUI::Immediate {

void EnqueueAdditionalFrame() {
  immediate_detail::tNeedAdditionalFrame.Set();
}

}// namespace FredEmmott::GUI::Immediate
