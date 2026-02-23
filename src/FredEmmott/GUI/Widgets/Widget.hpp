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

#include "FredEmmott/GUI/events/TextInputEvent.hpp"

namespace FredEmmott::GUI {
struct KeyEvent;
struct KeyReleaseEvent;
struct KeyPressEvent;
class Window;
}// namespace FredEmmott::GUI
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
    StyleClass primaryClass,
    const ImmutableStyle&,
    const StyleClasses& = {});
  virtual ~Widget();

  std::optional<MouseEvent> mWasStationaryHovered;

  void AddStyleClass(StyleClass);
  void ToggleStyleClass(StyleClass, bool value);

  // Can return nullptr
  [[nodiscard]]
  static Widget* FromYogaNode(YGNodeConstRef);
  Widget* GetParent() const;

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
  T* GetContext() const {
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

  [[nodiscard]]
  bool IsHovered() const;

  // A periodic event at an undefined interval; use for animations etc
  virtual void Tick(const std::chrono::steady_clock::time_point& now);
  void ComputeStyles(const Style& inherited);
  Style FlattenStyles(const Style&);

  /// User-provided styles
  void SetMutableStyles(const Style& styles);
  void AddMutableStyles(const Style& styles);

  void Paint(Renderer* renderer) const;

  [[nodiscard]]
  auto GetChildren() const noexcept {
    const auto foster = this->GetFosterParent();
    return (foster ? foster : this)->mRawDirectChildren;
  }
  Widget* SetChildren(const std::vector<Widget*>& children);

  /// Returns the Widget that ultimately handled the event, or nullptr
  [[nodiscard]]
  Widget* DispatchEvent(const Event&);

  const Style& GetComputedStyle() const {
    return mComputedStyle;
  }

  [[nodiscard]]
  Point GetTopLeftCanvasPoint(const Widget* relativeTo = nullptr) const;

  [[nodiscard]]
  Window* GetOwnerWindow() const noexcept {
    return mOwnerWindow;
  }

  [[nodiscard]]
  std::string GetAccessibilityName() const;

 protected:
  enum class StateFlags {
    Default = 0,
    Disabled = 1 << 1,
    Hovered = 1 << 2,
    Active = 1 << 3,
    Animating = 1 << 4,
    Checked = 1 << 5,
    HaveFocus = 1 << 6,
    HaveVisibleFocus = 1 << 7,
  };
  friend consteval bool is_bitflag_enum(std::type_identity<StateFlags>);

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
  friend consteval bool is_bitflag_enum(std::type_identity<ComputedStyleFlags>);

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

  // These are 'fire-and-forget' events which are separately dispatched for
  // each widget; as such, there's no propagation behavior to consider, so it
  // doesn't need to be returned
  virtual void OnMouseEnter(const MouseEvent&);
  virtual void OnMouseLeave(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseButtonPress(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseButtonRelease(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseMove(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseHover(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseVerticalWheel(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnMouseHorizontalWheel(const MouseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnKeyPress(const KeyPressEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnKeyRelease(const KeyReleaseEvent&);
  [[nodiscard]]
  virtual EventHandlerResult OnTextInput(const TextInputEvent&);

  [[nodiscard]] auto GetMutableStyles() const noexcept {
    return mMutableStyles;
  }

  /** Parent node for `GetChildren()` and `SetChildren()` (public APIs).
   *
   * Use `SetDirectChildren()` for internal sub-widgets
   */
  virtual Widget* GetFosterParent() const noexcept {
    return nullptr;
  }

  void StartMouseCapture();
  void EndMouseCapture();

  [[nodiscard]]
  bool IsChecked() const noexcept;
  void SetIsChecked(bool);

  void SetDirectChildren(const std::vector<Widget*>& children);

 private:
  struct MouseEventResult {
    EventHandlerResult mResult {EventHandlerResult::Default};
    Widget* mTarget {nullptr};
  };
  struct StyleTransitions;
  Window* mOwnerWindow {};
  StyleClass mPrimaryClass;

  unique_ptr<StyleTransitions> mStyleTransitions;

  ImmutableStyle mImmutableStyle;
  const std::size_t mID {};
  StyleClasses mClassList;
  unique_ptr<YGNode> mYoga;

  StateFlags mDirectStateFlags {};
  StateFlags mInheritedStateFlags {};
  Style mMutableStyles {};

  std::string mStylesCacheKey;
  Style mInheritedStyles;
  Style mComputedStyle;

  std::vector<unique_ptr<Widget>> mDirectChildren;
  std::vector<Widget*> mRawDirectChildren;

  std::unordered_map<std::type_index, std::unique_ptr<Context>> mContexts;

  Point mMouseCaptureOffset {};

  // Returns the innermost widget that received the event.
  [[nodiscard]]
  MouseEventResult DispatchMouseEvent(const MouseEvent&);
  Widget* DispatchKeyEvent(const KeyEvent&);
  Widget* DispatchTextInputEvent(const TextInputEvent&);

  [[nodiscard]]
  bool MatchesStylePseudoClass(StyleClass) const;
  [[nodiscard]]
  bool MatchesStyleClass(const StyleClass&) const;
  [[nodiscard]]
  bool MatchesStyleSelector(Style::Selector) const;
};

consteval bool is_bitflag_enum(std::type_identity<Widget::StateFlags>) {
  return true;
}

consteval bool is_bitflag_enum(std::type_identity<Widget::ComputedStyleFlags>) {
  return true;
}

}// namespace FredEmmott::GUI::Widgets