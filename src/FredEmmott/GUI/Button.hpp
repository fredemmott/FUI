// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI {

class Button final : public Widget {
 public:
  struct Options {};

  Button(std::size_t id, const Options&, Widget* label);

  void Paint(SkCanvas* canvas) const override;

 private:
  Widget* mLabel { nullptr };

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI
