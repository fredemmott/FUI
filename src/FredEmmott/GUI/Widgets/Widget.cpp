// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

Widget::Widget(std::size_t id) : mID(id), mYoga(YGNodeNew()) {
}

Widget::~Widget() = default;

}// namespace FredEmmott::GUI::Widgets