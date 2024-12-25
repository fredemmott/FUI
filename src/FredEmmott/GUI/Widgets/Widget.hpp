// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <FredEmmott/GUI/Widgets/WidgetStyles.hpp>
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

  void ComputeStyles(const WidgetStyles& inherited);

  void SetExplicitStyles(const WidgetStyles& styles);
  void Paint(SkCanvas* canvas) const;

  std::span<Widget* const> GetChildren() const noexcept;
  void SetChildren(const std::vector<Widget*>& children);

  void DispatchEvent(const Event*);

 protected:
  enum class EventHandlerResult {
    Default,
    StopPropagation,
  };
  enum class ComputedStyleFlags {
    Default = 0,
    InheritableHoverState = 1,
    InheritableActiveState = 2,
  };
  friend consteval bool is_bitflag_enum(
    utility::type_tag_t<ComputedStyleFlags>);

  // Base spacing unit - see https://fluent2.microsoft.design/layout
  static constexpr SkScalar Spacing = 4;

  explicit Widget(std::size_t id);

  [[nodiscard]]
  virtual WidgetStyles GetDefaultStyles() const {
    return {};
  }

  [[nodiscard]]
  virtual ComputedStyleFlags OnComputedStyleChange(const Style& base) {
    return ComputedStyleFlags::Default;
  }

  virtual void PaintOwnContent(SkCanvas*, const SkRect&, const Style& style)
    const {
  }

  [[nodiscard]]
  virtual EventHandlerResult OnClick(MouseEvent* event) {
    return EventHandlerResult::Default;
  }

  [[nodiscard]] auto GetExplicitStyles() const noexcept {
    return mExplicitStyles;
  }

 private:
  enum class StateFlags {
    Default = 0,
    Disabled = 1,
    MouseDownTarget = 1 << 1,
    Hovered = 1 << 2,
    HoveredInherited = 1 << 3,
    Active = 1 << 4,
    ActiveInherited = 1 << 5,
  };
  friend consteval bool is_bitflag_enum(utility::type_tag_t<StateFlags>);

  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;
  StateFlags mStateFlags {};
  WidgetStyles mExplicitStyles {};

  WidgetStyles mInheritedStyles;
  Style mComputedStyle;

  std::vector<unique_ptr<Widget>> mChildren;
  std::vector<Widget*> mStorageForGetChildren;

  [[nodiscard]]
  EventHandlerResult DispatchMouseEvent(const MouseEvent*);
};

consteval bool is_bitflag_enum(utility::type_tag_t<Widget::StateFlags>) {
  return true;
}

consteval bool is_bitflag_enum(
  utility::type_tag_t<Widget::ComputedStyleFlags>) {
  return true;
}

}// namespace FredEmmott::GUI::Widgets