// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/Widgets/WidgetStyles.hpp>
#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/events/MouseMoveEvent.hpp>

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

  FrameRateRequirement GetFrameRateRequirement() const noexcept;

  /// Whether this widget is disabled, including by a parent
  [[nodiscard]]
  bool IsDisabled() const;
  [[nodiscard]]
  bool IsDirectlyDisabled() const;
  void SetIsDirectlyDisabled(bool value);

  void ComputeStyles(const WidgetStyles& inherited);

  /// User-provided styles
  void SetExplicitStyles(const WidgetStyles& styles);
  // For immediate API - fake a widget by replacing its built-in styles
  void SetBuiltInStyles(const WidgetStyles& styles);
  void SetAdditionalBuiltInStyles(const WidgetStyles& styles);
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
    InheritableHoverState = 1 << 0,
    InheritableActiveState = 1 << 1,
    Animating = 1 << 2,
  };
  friend consteval bool is_bitflag_enum(
    utility::type_tag_t<ComputedStyleFlags>);

  // Base spacing unit - see https://fluent2.microsoft.design/layout
  static constexpr SkScalar Spacing = 4;

  [[nodiscard]]
  virtual WidgetStyles GetBuiltInStyles() const {
    return {};
  }

  [[nodiscard]]
  virtual ComputedStyleFlags OnComputedStyleChange(const Style& style);

  virtual void PaintOwnContent(SkCanvas*, const SkRect&, const Style& style)
    const {}

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
    MouseDownTarget = 1,
    Disabled = 1 << 2,
    Hovered = 1 << 3,
    Active = 1 << 4,
    Animating = 1 << 5,
  };
  friend consteval bool is_bitflag_enum(utility::type_tag_t<StateFlags>);

  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;
  StateFlags mDirectStateFlags {};
  StateFlags mInheritedStateFlags {};
  WidgetStyles mExplicitStyles {};
  std::optional<WidgetStyles> mReplacedBuiltInStyles;

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