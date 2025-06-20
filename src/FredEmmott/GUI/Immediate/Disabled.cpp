// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Disabled.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginDisabled(const bool isDisabled, const ID id) {
  static const Style baseStyles {.mDisplay = YGDisplayContents};
  using namespace immediate_detail;
  BeginWidget<Widgets::Widget>(id);
  auto widget = GetCurrentParentNode();
  widget->ReplaceExplicitStyles(baseStyles);
  widget->SetIsDirectlyDisabled(isDisabled);
}

}// namespace FredEmmott::GUI::Immediate