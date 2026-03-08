// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBlock.hpp"

#include <FredEmmott/GUI/Widgets/TextBlock.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {

TextBlockResult TextBlock(const std::string_view text, const ID id) {
  using Widgets::TextBlock;
  using namespace immediate_detail;

  const auto widget = ChildlessWidget<TextBlock>(id);
  widget->SetText(text);

  return {widget};
}

}// namespace FredEmmott::GUI::Immediate
