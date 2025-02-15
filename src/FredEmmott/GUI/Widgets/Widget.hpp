// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkCanvas.h>

#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/StyleSheet.hpp>
#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/events/MouseMoveEvent.hpp>
#include <FredEmmott/GUI/yoga.hpp>

namespace FredEmmott::GUI::Widgets {
using namespace FredEmmott::Memory;

struct WidgetList;

class Widget {
 public:
  Widget() = delete;
  Widget(std::size_t id, const StyleClasses& = {});
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

  void AddStyleClass(const StyleClass&);
  void ToggleStyleClass(const StyleClass&, std::optional<bool> value);

  void ComputeStyles(
    const StyleSheet&,
    const std::vector<const Widget*>& ancestors,
    const Style& inherited);

  /// User-provided styles
  void SetExplicitStyles(const Style& styles);
  // For immediate API - fake a widget by replacing its built-in styles
  void ReplaceBuiltInStyleSheet(const StyleSheet&);
  void AppendBuiltInStyleSheet(const StyleSheet&);
  void AppendBuiltInStyles(const Style&);
  void Paint(SkCanvas* canvas) const;

  auto GetChildren() const noexcept {
    const auto foster = this->GetFosterParent();
    return (foster ? foster : this)->mManagedChildrenCacheForGetChildren;
  }
  void SetChildren(const std::vector<Widget*>& children);

  void DispatchEvent(const Event*);

  [[nodiscard]]
  bool HasStyleClass(StyleClass) const;

 protected:
  enum class StateFlags {
    Default = 0,
    Disabled = 1 << 1,
    Hovered = 1 << 2,
    Active = 1 << 3,
    Animating = 1 << 4,
  };
  friend consteval bool is_bitflag_enum(utility::type_tag_t<StateFlags>);

  enum class EventHandlerResult {
    Default,
    StopPropagation,
  };
  enum class ComputedStyleFlags {
    Empty = 0,
    InheritableHoverState = 1 << 0,
    InheritableActiveState = 1 << 1,
    Animating = 1 << 2,
  };
  friend consteval bool is_bitflag_enum(
    utility::type_tag_t<ComputedStyleFlags>);

  // Base spacing unit - see https://fluent2.microsoft.design/layout
  static constexpr SkScalar Spacing = 4;

  [[nodiscard]]
  virtual Style GetBuiltInStyles_DEPRECATED() const {
    return {};
  }
  [[nodiscard]]
  virtual StyleSheet GetBuiltInStyleSheet() const {
    if (mReplacedBuiltInStyleSheet) {
      return *mReplacedBuiltInStyleSheet;
    } else {
      return ConvertLegacyStylesToStyleSheet(GetBuiltInStyles_DEPRECATED());
    }
  }

  [[nodiscard]]
  StyleSheet ConvertLegacyStylesToStyleSheet(const Style&) const;

  [[nodiscard]]
  virtual ComputedStyleFlags OnComputedStyleChange(
    const Style& style,
    StateFlags state);

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

  StyleClasses mClassList;
  const std::size_t mID {};
  unique_ptr<YGNode> mYoga;

  StateFlags mDirectStateFlags {};
  StateFlags mInheritedStateFlags {};
  Style mExplicitStyles {};
  std::optional<StyleSheet> mReplacedBuiltInStyleSheet;

  Style mComputedStyle;

  std::vector<unique_ptr<Widget>> mManagedChildren;
  std::vector<Widget*> mManagedChildrenCacheForGetChildren;

  [[nodiscard]]
  EventHandlerResult DispatchMouseEvent(const MouseEvent*);
  void SetManagedChildren(const std::vector<Widget*>& children);
  [[nodiscard]]
  bool MatchesStylePseudoClass(StyleClass) const;
};

consteval bool is_bitflag_enum(utility::type_tag_t<Widget::StateFlags>) {
  return true;
}

consteval bool is_bitflag_enum(
  utility::type_tag_t<Widget::ComputedStyleFlags>) {
  return true;
}

}// namespace FredEmmott::GUI::Widgets