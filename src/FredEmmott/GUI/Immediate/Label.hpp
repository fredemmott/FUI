// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Label.hpp>
#include <FredEmmott/GUI/Immediate/Leaf.hpp>

namespace FredEmmott::GUI::Immediate {
using LabelOptions = Widgets::Label::Options;

static constexpr Leaf<Widgets::Label> Label;
}
