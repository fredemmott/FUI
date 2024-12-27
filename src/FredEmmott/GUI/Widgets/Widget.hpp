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

struct WidgetList;

class Widget {
 public:
  Widget() = delete;
  explicit Widget(std::size_t id);
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

  auto GetChildren() const noexcept {
    const auto foster = this->GetFosterParent();
    return (foster ? foster : this)->mManagedChildrenCacheForGetChildren;
  }
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

  [[nodiscard]]
  virtual WidgetStyles GetDefaultStyles() const {
    return {};
  }

  [[nodiscard]]
  virtual ComputedStyleFlags OnComputedStyleChange(const Style& style);

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

  /** Parent node for `GetChildren()` and `SetChildren()` (public APIs).
   *
   * Use `GetDirectChildren()` and `ChangeDirectChildren()` for internal
   * sub-widgets.
   */
  virtual Widget* GetFosterParent() const noexcept {
    return nullptr;
  }

  virtual WidgetList GetDirectChildren() const noexcept;
  void ChangeDirectChildren(const std::function<void()>& mutator);

 private:
  struct StyleTransitions;
  unique_ptr<StyleTransitions> mStyleTransitions;

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

  std::vector<unique_ptr<Widget>> mManagedChildren;
  std::vector<Widget*> mManagedChildrenCacheForGetChildren;

  [[nodiscard]]
  EventHandlerResult DispatchMouseEvent(const MouseEvent*);
  void SetManagedChildren(const std::vector<Widget*>& children);
};

consteval bool is_bitflag_enum(utility::type_tag_t<Widget::StateFlags>) {
  return true;
}

consteval bool is_bitflag_enum(
  utility::type_tag_t<Widget::ComputedStyleFlags>) {
  return true;
}

}// namespace FredEmmott::GUI::Widgets