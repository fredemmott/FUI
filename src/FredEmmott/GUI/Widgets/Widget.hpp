// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/events/MouseMoveEvent.hpp>
#include <span>

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

  std::size_t GetID() const noexcept {
    return mID;
  }

  virtual void Paint(SkCanvas*) const = 0;

  virtual std::span<Widget* const> GetChildren() const noexcept;

  void DispatchEvent(const Event*);

  [[nodiscard]] bool IsHovered() const noexcept;

 protected:
  // Base spacing unit - see https://fluent2.microsoft.design/layout
  static constexpr SkScalar Spacing = 4;

  explicit Widget(std::size_t id);

 private:
  enum class StateFlags {
    Default = 0,
    Disabled = 1,
    Hovered = 2,
  };
  friend consteval bool is_bitflag_enum(utility::type_tag_t<StateFlags>);

  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;
  StateFlags mStateFlags {};

  void DispatchMouseEvent(const MouseEvent*);
};

consteval bool is_bitflag_enum(utility::type_tag_t<Widget::StateFlags>) {
  return true;
}

}// namespace FredEmmott::GUI::Widgets