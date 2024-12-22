// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"


namespace FredEmmott::GUI {

Widget::Widget(std::size_t id): mID(id), mYoga(YGNodeNew()) {
}

std::size_t Widget::StringToId(std::string_view label) {
  auto separator = label.rfind(label);
  if (separator == std::string_view::npos) {
    return std::hash<std::string_view>{}(label);
  }
  return std::hash<std::string_view>{}(label.substr(separator + 2));
}

Widget::~Widget() = default;

}