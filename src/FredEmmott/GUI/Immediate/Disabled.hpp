// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

void BeginDisabled(
  bool isDisabled = true,
  const Widgets::WidgetStyles& styles = {});

void EndDisabled();

inline void BeginEnabled(
  bool isEnabled = true,
  const Widgets::WidgetStyles& styles = {}) {
  BeginDisabled(!isEnabled, styles);
}

inline void EndEnabled() {
  EndDisabled();
}

}// namespace FredEmmott::GUI::Immediate