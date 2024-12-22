// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI {
class Font;
}

namespace FredEmmott::GUI::WidgetFont {
enum class Usage {
  ControlContent,
};
using enum Usage;
Font Resolve(Usage usage) noexcept;

}