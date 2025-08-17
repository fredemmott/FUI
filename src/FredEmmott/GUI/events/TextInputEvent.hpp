// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string_view>

#include "Event.hpp"

namespace FredEmmott::GUI {

struct TextInputEvent final : Event {
  TextInputEvent() = default;
  ~TextInputEvent() override = default;

  constexpr explicit TextInputEvent(std::string_view text) : mText(text) {}
  std::string_view mText;
};
}// namespace FredEmmott::GUI