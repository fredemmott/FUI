// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <ranges>
#include <vector>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Layout : public Widget {
 public:
  std::span<Widget* const> GetChildren() const noexcept override {
    return std::span {mChildRawPointers.data(), mChildRawPointers.size()};
  }
  void SetChildren(const std::vector<Widget*>& children);

  void Paint(SkCanvas*) const override;

 protected:
  using Widget::Widget;

 private:
  std::vector<unique_ptr<Widget>> mChildren;
  // Storage for `GetChildren()`
  std::vector<Widget*> mChildRawPointers;
};

}// namespace FredEmmott::GUI::Widgets