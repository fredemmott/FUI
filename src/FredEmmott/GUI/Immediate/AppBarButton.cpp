// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "AppBarButton.hpp"

#include "FredEmmott/GUI/Widgets/AppBarButton.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {
AppBarButtonResult AppBarButton(
  const std::string_view glyph,
  const std::string_view label,
  const ID id) {
  const auto w = immediate_detail::ChildlessWidget<Widgets::AppBarButton>(id);
  w->SetGlyph(glyph);
  w->SetLabel(label);
  return {w, w->ConsumeWasActivated()};
}
}// namespace FredEmmott::GUI::Immediate
