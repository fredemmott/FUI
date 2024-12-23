// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button final : public Widget {
 public:
  struct Options {
    Style mStyle;
    Style mHoverStyle;
  };

  Button(std::size_t id, const Options&);

  void Paint(SkCanvas* canvas) const override;
  Widget* GetChild() const noexcept;
  void SetChild(Widget*);
  std::span<Widget* const> GetChildren() const noexcept override;

 private:
  Options mOptions;

  unique_ptr<Widget> mLabel {nullptr};
  // Lazy-initialized storage for `GetChildren()`'s span
  mutable Widget* mLabelRawPointer {nullptr};

  void SetLayoutConstraints();
  Style GetStyle() const noexcept;
};

}// namespace FredEmmott::GUI::Widgets
