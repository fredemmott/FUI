// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Disabled.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

Result<&EndDisabled, void, immediate_detail::WidgetlessResultMixin>
BeginDisabled(const bool isDisabled, const ID id) {
  static const auto PrimaryClass = StyleClass::Make("conditionally-disabled");
  static const ImmutableStyle Styles {
    Style().Display(YGDisplayContents),
  };

  using namespace immediate_detail;
  const auto widget = BeginWidget<Widgets::Widget>(id, PrimaryClass, Styles);
  widget->SetIsDirectlyDisabled(isDisabled);
  return {};
}

}// namespace FredEmmott::GUI::Immediate