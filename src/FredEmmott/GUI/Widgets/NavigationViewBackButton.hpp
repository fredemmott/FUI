// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "NavigationViewButton.hpp"

namespace FredEmmott::GUI::Widgets {

class NavigationViewBackButton final : public NavigationViewButton {
 public:
  NavigationViewBackButton();
  ~NavigationViewBackButton() override;

  void Invoke() override;

  [[nodiscard]]
  bool ConsumeWasActivated() noexcept {
    return std::exchange(mWasActivated, false);
  }

 private:
  bool mWasActivated {false};
};

}// namespace FredEmmott::GUI::Widgets