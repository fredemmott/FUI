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
    return std::ranges::ref_view(mChildren);
  }

  void SetChildren(std::vector<Widget*>&& children);
  void AppendChild(Widget* child);

  void Paint(SkCanvas*) const override;

 protected:
  using Widget::Widget;

 private:
  std::vector<Widget*> mChildren;
};

}// namespace FredEmmott::GUI::Widgets