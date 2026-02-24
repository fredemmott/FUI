// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "FredEmmott/GUI/Immediate/Result.hpp"
#include "FredEmmott/GUI/Widgets/TextBox.hpp"
#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndTextBox() {
  immediate_detail::EndWidget<Widgets::TextBox>();
}

using TextBoxResult = Result<
  &EndTextBox,
  bool,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin>;

[[nodiscard]]
TextBoxResult TextBox(
  std::string* text,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate
