// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "../Color.hpp"
#include "../WidgetColor.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Card final : public Widget {
 public:
  Card(std::size_t id);

  Widget* GetChild() const noexcept;
  void SetChild(Widget* child);
  std::span<Widget* const> GetChildren() const noexcept override;

 protected:
  WidgetStyles GetDefaultStyles() const override;

 private:
  unique_ptr<Widget> mChild;
  // Lazy-initialized storage for `GetChildren()`'s span
  mutable Widget* mChildRawPointer;
};

}// namespace FredEmmott::GUI::Widgets