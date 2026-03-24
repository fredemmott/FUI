// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "NavigationViewButton.hpp"

namespace FredEmmott::GUI::Widgets {

class NavigationView;

class NavigationViewTogglePaneButton final : public NavigationViewButton {
 public:
  explicit NavigationViewTogglePaneButton(NavigationView*);
  ~NavigationViewTogglePaneButton() override;

  void Invoke() override;

 private:
  NavigationView* mNavigationView {};
};

}// namespace FredEmmott::GUI::Widgets