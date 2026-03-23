// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI.hpp>

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

fuii::VStackPanelResult BeginDemoPage();
fuii::CardResult BeginDemoCard(
  fuii::ID = fuii::ID {std::source_location::current()});

#ifdef _WIN32
void demo_win32();
#endif