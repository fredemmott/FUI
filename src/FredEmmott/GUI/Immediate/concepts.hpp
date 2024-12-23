// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <concepts>

namespace FredEmmott::GUI::Immediate {

template <class T>
concept widget = std::derived_from<T, Widgets::Widget>;

template <class T>
concept string_widget = widget<T> && requires(T widget) {
  widget.SetText("abc");
  { widget.GetText() } -> std::convertible_to<std::string_view>;
};

template <class T>
concept single_child_widget
  = widget<T> && requires(T widget, Widgets::Widget* child) {
      widget.SetChild(child);
      { widget.GetChild() } -> std::convertible_to<Widgets::Widget*>;
    };

template <class T>
concept multi_child_widget
  = widget<T> && requires(T widget, std::vector<Widgets::Widget*> children) {
      widget.AppendChild(children.front());
      widget.SetChildren(std::move(children));
      { widget.GetChildren() } -> std::ranges::input_range;
      { *begin(widget.GetChildren()) } -> std::convertible_to<Widgets::Widget*>;
    };

}// namespace FredEmmott::GUI::Immediate