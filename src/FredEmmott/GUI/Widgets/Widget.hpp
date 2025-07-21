// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/Rect.hpp>
#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/events/Event.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>
#include <FredEmmott/GUI/yoga.hpp>
#include <typeindex>

namespace FredEmmott::GUI::Widgets {
using namespace FredEmmott::Memory;

struct WidgetList;

/** Subclass to attach arbitrary data to a widget.
 *
 * @see `Widget::SetContextIfUnset()`
 * @see `Widget::GetContext()`
 */
class Context {
 public:
  virtual ~Context() = default;
};

template <class T>
concept context = std::derived_from<T, Context>;

class Widget {
 public:
  Widget() = delete;
  Widget(const Widget&) = delete;
  Widget(Widget&&) = delete;
  Widget& operator=(const Widget&) = delete;
  Widget& operator=(Widget&&) = delete;

  explicit Widget(
    std::size_t id,
    const ImmutableStyle&,
    const StyleClasses& = {});
  virtual ~Widget();

  void AddStyleClass(StyleClass);
  void ToggleStyleClass(StyleClass, bool value);

  // Can return nullptr
  [[nodiscard]]
  static Widget* FromYogaNode(YGNodeConstRef);

  [[nodiscard]] YGNodeRef GetLayoutNode() const noexcept {
    return mYoga.get();
  }

  std::size_t GetID() const noexcept {
    return mID;
  }

  /** Attach user-supplied data, derived from the `Context` class.
   *
   * Retrieve with `GetContext()`
   */
  template <
    std::invocable F,
    context T = typename std::invoke_result_t<F>::element_type>
    requires std::same_as<std::invoke_result_t<F>, std::unique_ptr<T>>
  void SetContextIfUnset(F&& f) {
    const auto key = std::type_index(typeid(T));
    if (mContexts.contains(key)) {
      return;
    }
    mContexts.emplace(key, std::invoke(std::forward<F>(f)));
  }

  template <context T, class... Args>
    requires std::constructible_from<T, Args...>
  void SetContextIfUnset(Args&&... args) {
    const auto key = std::type_index(typeid(T));
    if (mContexts.contains(key)) {
      return;
    }
    mContexts.emplace(key, std::make_unique<T>(std::forward<Args>(args)...));
  }

  /** Retrieve user-supplied data, derived from the `Context` class.
   *
   * Set with `SetContextIfUnset()`
   */
  template <context T>
  T* GetContext() {
    const auto key = std::type_index(typeid(T));
    if (!mContexts.contains(key)) {
      return nullptr;
    }
    return static_cast<T*>(mContexts.at(key).get());
  }

  template <
    std::invocable F,
    context T = typename std::invoke_result_t<F>::element_type>
    requires std::same_as<std::invoke_result_t<F>, std::unique_ptr<T>>
  T* GetOrCreateContext(F&& f) {
    SetContextIfUnset(std::forward<F>(f));
    return GetContext<T>();
  }

  template <context T, class... Args>
    requires std::constructible_from<T, Args...>
  T* GetOrCreateContext(Args&&... args) {
    const auto key = std::type_index(typeid(T));
    if (mContexts.contains(key)) {
      return static_cast<T*>(mContexts.at(key).get());
    }
    auto owned = std::make_unique<T>(std::forward<Args>(args)...);
    const auto ret = owned.get();
    mContexts.emplace(key, std::move(owned));
    return ret;
  }

  virtual FrameRateRequirement GetFrameRateRequirement() const noexcept;

  /// Whether this widget is disabled, including by a parent
  [[nodiscard]]
  bool IsDisabled() const;
  [[nodiscard]]
  bool IsDirectlyDisabled() const;
  void SetIsDirectlyDisabled(bool value);

  // A periodic event at an undefined interval; use for animations etc
  virtual void Tick();
  virtual void UpdateLayout();
  void ComputeStyles(const Style& inherited);
  Style FlattenStyles(const Style&);

  /// User-provided styles
  void ReplaceExplicitStyles(const Style& styles);
  void AddExplicitStyles(const Style& styles);

  void Paint(Renderer* renderer) const;

  [[nodiscard]]
  auto GetChildren() const noexcept {
    const auto foster = this->GetFosterParent();
    return (foster ? foster : this)->mManagedChildrenCacheForGetChildren;
  }
  void SetChildren(const std::vector<Widget*>& children);

  /// Returns the Widget that ultimately handled the event, or nullptr
  [[nodiscard]]
  Widget* DispatchEvent(const Event*);

  const Style& GetComputedStyle() const {
    return mComputedStyle;
  }

  Point GetTopLeftCanvasPoint() const;

 protected:
  enum class StateFlags {
    Default = 0,
    Disabled = 1 << 1,
    Hovered = 1 << 2,
    Active = 1 << 3,
    Animating = 1 << 4,
    Checked = 1 << 5,
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

  [[nodiscard]]
  virtual ComputedStyleFlags OnComputedStyleChange(
    const Style& style,
    StateFlags state);

  virtual void PaintOwnContent(Renderer*, const Rect&, const Style&) const {}
  virtual void PaintChildren(Renderer* canvas) const;

  [[nodiscard]]
  virtual EventHandlerResult OnClick(const MouseEvent&) {
    return EventHandlerResult::Default;
  }

  [[nodiscard]]
  virtual EventHandlerResult OnMouseButtonPress(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseButtonRelease(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseMove(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseVerticalWheel(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseHorizontalWheel(const MouseEvent&);

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

  void StartMouseCapture();
  void EndMouseCapture();

  [[nodiscard]]
  bool IsChecked() const noexcept;
  void SetIsChecked(bool);

 private:
  struct MouseEventResult {
    EventHandlerResult mResult {EventHandlerResult::Default};
    Widget* mTarget {nullptr};
  };
  struct StyleTransitions;
  unique_ptr<StyleTransitions> mStyleTransitions;

  ImmutableStyle mImmutableStyle;
  const std::size_t mID {};
  StyleClasses mClassList;
  unique_ptr<YGNode> mYoga;

  StateFlags mDirectStateFlags {};
  StateFlags mInheritedStateFlags {};
  Style mExplicitStyles {};

  Style mInheritedStyles;
  Style mComputedStyle;

  std::vector<unique_ptr<Widget>> mManagedChildren;
  std::vector<Widget*> mManagedChildrenCacheForGetChildren;

  std::unordered_map<std::type_index, std::unique_ptr<Context>> mContexts;

  Point mMouseOffset {};

  // Returns the innermost widget that received the event.
  [[nodiscard]]
  MouseEventResult DispatchMouseEvent(const MouseEvent&);
  void SetManagedChildren(const std::vector<Widget*>& children);

  [[nodiscard]]
  bool MatchesStylePseudoClass(StyleClass) const;
  [[nodiscard]]
  bool MatchesStyleClass(const StyleClass&) const;
  [[nodiscard]]
  bool MatchesStyleSelector(Style::Selector) const;
};

consteval bool is_bitflag_enum(utility::type_tag_t<Widget::StateFlags>) {
  return true;
}

consteval bool is_bitflag_enum(
  utility::type_tag_t<Widget::ComputedStyleFlags>) {
  return true;
}

}// namespace FredEmmott::GUI::Widgets