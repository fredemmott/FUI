// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <format>
#include <string>

#include "../yoga.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace FredEmmott::Memory;

class Widget {
 public:
  Widget() = delete;
  virtual ~Widget();

  [[nodiscard]] YGNodeRef GetLayoutNode() const noexcept {
    return mYoga.get();
  }

  virtual void Paint(SkCanvas*) const = 0;

 protected:
  // Base spacing unit - see https://fluent2.microsoft.design/layout
  static constexpr SkScalar Spacing = 4;

  explicit Widget(std::size_t id);

 private:
  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;
};

}// namespace FredEmmott::GUI::Widgets