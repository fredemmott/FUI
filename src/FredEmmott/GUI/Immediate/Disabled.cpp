// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Disabled.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

Result<&EndDisabled, void, immediate_detail::WidgetlessResultMixin>
BeginDisabled(const bool isDisabled, const ID id) {
  static const auto BaseStyles = Style().Display(YGDisplayContents);

  using namespace immediate_detail;
  BeginWidget<Widgets::Widget>(id);
  auto widget = GetCurrentParentNode();
  widget->ReplaceExplicitStyles(BaseStyles);
  widget->SetIsDirectlyDisabled(isDisabled);
  return {};
}

}// namespace FredEmmott::GUI::Immediate