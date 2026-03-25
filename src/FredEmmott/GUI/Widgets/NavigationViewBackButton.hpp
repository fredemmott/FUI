// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "NavigationViewButton.hpp"

namespace FredEmmott::GUI::Widgets {

class NavigationViewBackButton final : public NavigationViewButton {
 public:
  explicit NavigationViewBackButton(Window*);
  ~NavigationViewBackButton() override;

  void Invoke() override;
};

}// namespace FredEmmott::GUI::Widgets