// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Disabled.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginDisabled(bool isDisabled, const Widgets::WidgetStyles& styles) {
  static const Widgets::WidgetStyles baseStyles {
    .mBase = {
      .mDisplay = YGDisplayContents,
    },
  };

  immediate_detail::BeginWidget<Widgets::Widget> {}();
  auto widget = immediate_detail::GetCurrentParentNode();
  widget->SetExplicitStyles(baseStyles + styles);
  widget->SetIsDirectlyDisabled(isDisabled);
}

void EndDisabled() {
  immediate_detail::EndWidget<Widgets::Widget>();
}

}// namespace FredEmmott::GUI::Immediate