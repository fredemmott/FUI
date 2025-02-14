// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button final : public Widget {
 public:
  Button(std::size_t id);

  ActivatedFlag mClicked;

 protected:
  Style GetBuiltInStyles_DEPRECATED() const override;
  EventHandlerResult OnClick(MouseEvent* e) override;
};

}// namespace FredEmmott::GUI::Widgets
