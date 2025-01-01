// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/GUI/Widgets/Label.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

void Label(const std::string_view text, const ID id) {
  using Widgets::Label;
  using namespace immediate_detail;

  BeginWidget<Label>(id);
  GetCurrentParentNode<Label>()->SetText(text);
  EndWidget<Label>();
}
}// namespace FredEmmott::GUI::Immediate
