// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {

bool IsButtonClicked() {
  auto button = immediate_detail::GetCurrentParentNode<Widgets::Button>();
  if (!button) [[unlikely]] {
    throw std::logic_error(
      "IsButtonClick() called, but current node is not a button");
  }
  return button->mClicked.TestAndClear();
}

}// namespace FredEmmott::GUI::Immediate