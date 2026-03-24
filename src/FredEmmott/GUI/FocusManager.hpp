// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>
#include <tuple>

#include "FredEmmott/GUI/Widgets/Widget.hpp"
#include "Widgets/TextBox.hpp"

namespace FredEmmott::GUI {
/** Why/how a widget is focused.
 *
 * Each window has 0 or 1 focused widgets. If there is a focused widget,
 * it could be visibly focused or implicitly focused.
 */
enum class FocusKind {
  /* A widget visibly has focus.
   *
   * For example, it may have a white border.
   *
   * Usually triggered by keyboard interaction.
   *
   * For example, using tab or shift-tab to navigate widgets will result
   * in visible focus.
   *
   * In CSS terms, this corresponds to an element having _both_ the `:focus` and
   * `:focus-visible` pseudo-classes.
   */
  Visible,
  /* A widget has focus but does not necessarily have a visible indicator.
   *
   * For example, clicking on a button will give it implicit focus, but will
   * not result in drawing the white focus outline.
   *
   * While most widgets do not visually indicate this state, some may,
   * especially keyboard-centered widgets like text boxes.
   *
   * In CSS terms, this corresponds to an element having the `:focus`
   * pseudoclass, but _not_ having the `:focus-visible` pseudo-class.
   */
  Implicit,
};

class FocusManager final {
 public:
  FocusManager() = delete;
  ~FocusManager();

  explicit FocusManager(Widgets::Widget* rootWidget);

  [[nodiscard]] std::optional<std::tuple<Widgets::Widget*, FocusKind>>
  GetFocusedWidget() const;

  [[nodiscard]]
  static bool IsWidgetFocused(Widgets::Widget const*);

  void GiveImplicitFocus(Widgets::Widget*);
  void GiveVisibleFocus(Widgets::Widget*);
  void FocusNextWidget();
  void FocusPreviousWidget();

  void BeforeDestroy(Widgets::Widget*);

  [[nodiscard]]
  bool OnKeyPress(const KeyPressEvent& e);

  /// Thread-local
  static FocusManager* Get();

  static void PushInstance(FocusManager*);
  static void PopInstance(const FocusManager*);

 private:
  Widgets::Widget* mRootWidget {};
  Widgets::Widget* mFocusedWidget {};
  FocusKind mFocusKind {FocusKind::Implicit};

  void FocusFirstWidget();
  void FocusLastWidget();

  void FocusFirstSelectionItem(auto makeRange);

  void FocusNextSelectionItem();
  void FocusPreviousSelectionItem();

  void FocusFirstSelectedItem();

  static Widgets::Widget* FirstFocusableWidget(Widgets::Widget* parent);
  static Widgets::Widget* LastFocusableWidget(Widgets::Widget* parent);
};

}// namespace FredEmmott::GUI