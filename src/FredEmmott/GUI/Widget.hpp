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
  // Base spacing unit - see https://fluent2.microsoft.design/layout
  static constexpr SkScalar Spacing = 4;

  explicit Widget(std::size_t id);
  explicit Widget(std::string_view id) : Widget(StringToId(id)) {
  }

  struct FormattedLabel {
    std::size_t mID;
    std::string mText;

    FormattedLabel() = delete;

    template <class... Args>
    explicit FormattedLabel(std::format_string<Args...> format, Args&&... args) {
      const auto formatted = std::format(format, std::forward<Args>(args)...);

      {
        const std::string_view view = format.get();
        const auto i = view.rfind("##");
        if (i == std::string_view::npos) {
          mID = std::hash<std::string_view> {}(formatted);
          mText = formatted;
          return;
        }
      }

      const auto i = formatted.rfind("##");
      mID = std::hash<std::string_view> {}(
        std::string_view {formatted}.substr(i + 2));
      mText = formatted.substr(0, i);
    }
  };

  static constexpr FormattedLabel FormatLabel(
    const auto& format,
    const std::string_view formatted) {
    const auto formatView = format.get();
    const auto idx = formatView.rfind("##");
    if (idx != std::string_view::npos) {
      return std::hash<std::string_view> {}(formatView.substr(idx + 2));
    }
    return std::hash<std::string_view> {}(formatted);
  }

 private:
  static std::size_t StringToId(std::string_view label);

  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;
};

}// namespace FredEmmott::GUI