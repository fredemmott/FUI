// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button final : public Widget {
 public:
  Button(std::size_t id);

 protected:
  WidgetStyles GetDefaultStyles() const override;
};

}// namespace FredEmmott::GUI::Widgets
