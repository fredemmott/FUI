// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button final : public Widget {
 public:
  Button(std::size_t id);

  Widget* GetChild() const noexcept;
  void SetChild(Widget*);
  std::span<Widget* const> GetChildren() const noexcept override;

 protected:
  WidgetStyles GetDefaultStyles() const override;

 private:
  unique_ptr<Widget> mLabel {nullptr};
  // Lazy-initialized storage for `GetChildren()`'s span
  mutable Widget* mLabelRawPointer {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
