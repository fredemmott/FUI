// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <format>
#include <string>

#include "yoga.hpp"

namespace FredEmmott::GUI {
using namespace FredEmmott::Memory;

class SystemColor;

class Widget {
 public:
  Widget() = delete;
  virtual ~Widget();

  [[nodiscard]] YGNodeRef GetLayoutNode() const noexcept {
    return mYoga.get();
  }

  virtual void Paint(SkCanvas*) const = 0;

 protected:
  explicit Widget(std::size_t id);
  explicit Widget(std::string_view id) : Widget(StringToId(id)) {
  }

  static constexpr std::size_t FormattedToId(
    const auto& format,
    const std::string_view formatted) {
    const auto formatView = format.get();
    const auto idx = formatView.rfind("##");
    if (idx != std::string_view::npos) {
      return std::hash<std::string_view>{}(formatView.substr(idx + 2));
    }
      return std::hash<std::string_view>{}(formatted);
  }
 private:
  static std::size_t StringToId(std::string_view label);

  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;
};

}// namespace FredEmmott::GUI