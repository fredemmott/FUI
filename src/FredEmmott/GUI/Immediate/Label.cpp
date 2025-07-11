// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/GUI/Widgets/Label.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {

LabelResult Label(const std::string_view text, const ID id) {
  using Widgets::Label;
  using namespace immediate_detail;

  const auto label = BeginWidget<Label>(id);
  label->SetText(text);
  EndWidget<Label>();
  return {label};
}
}// namespace FredEmmott::GUI::Immediate
