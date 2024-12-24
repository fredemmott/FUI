// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>
#include <FredEmmott/GUI/Widgets/Label.hpp>


namespace FredEmmott::GUI::Immediate {

  template <class... Args>
  void Label(
    const Widgets::WidgetStyles& styles,
    std::format_string<Args...> fmt,
    Args&&... args) {
    using Widgets::Label;
    using namespace immediate_detail;

    const auto [id, text] = ParsedID::Make<Label>(fmt, std::forward<Args>(args)...);

    BeginWidget<Label> {}(styles, id);
    GetCurrentParentNode<Label>()->SetText(text);
    EndWidget();
  }

  template <class... Args>
  void Label(std::format_string<Args...> fmt, Args&&... args) {
    return Label({}, fmt, std::forward<Args>(args)...);
  }

}// namespace FredEmmott::GUI::Immediate