// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PushID.hpp"

#include "FredEmmott/GUI/Style.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {

void PopID() {
  immediate_detail::EndWidget<Widgets::Widget>();
}

Result<&PopID, void, immediate_detail::WidgetlessResultMixin> PushID(
  const ID id) {
  static const ImmutableStyle MyStyle {Style().Display(YGDisplayContents)};
  immediate_detail::BeginWidget<Widgets::Widget>(
    id, LiteralStyleClass {"id-scope"}, MyStyle);
  return {};
}

}// namespace FredEmmott::GUI::Immediate