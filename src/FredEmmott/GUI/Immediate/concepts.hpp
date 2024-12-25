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

}// namespace FredEmmott::GUI::Immediate