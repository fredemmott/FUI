// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <ranges>
#include <vector>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Layout : public Widget {
 public:
  auto GetChildren() const noexcept {
    return std::views::transform(mChildren, &unique_ptr<Widget>::get);
  }
  void SetChildren(const std::vector<Widget*>& children);

  void Paint(SkCanvas*) const override;

 protected:
  using Widget::Widget;

 private:
  std::vector<unique_ptr<Widget>> mChildren;
};

}// namespace FredEmmott::GUI::Widgets