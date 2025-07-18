// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PushID.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {

void PopID() {
  immediate_detail::EndWidget<Widgets::Widget>();
}

Result<&PopID, void, immediate_detail::WidgetlessResultMixin> PushID(
  const ID id) {
  auto w = immediate_detail::BeginWidget<Widgets::Widget>(id);
  w->SetBuiltInStyles(Style().Display(YGDisplayContents));
  return {};
}

}// namespace FredEmmott::GUI::Immediate