// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {

bool IsPreviousButtonClicked() {
  // TODO: when we track clicks in the button, we can pull this out of
  // `tStack`, and dynamic-cast the current sibling
  return false;
}

}// namespace FredEmmott::GUI::Immediate