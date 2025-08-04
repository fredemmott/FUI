// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>
#include <tuple>

#include "FredEmmott/GUI/Widgets/Widget.hpp"
#include "Widgets/TextBox.hpp"

namespace FredEmmott::GUI {
enum class FocusKind {
  Keyboard,
  Pointer,
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

  void GivePointerFocus(Widgets::Widget*);
  void FocusNextWidget();
  void FocusPreviousWidget();

  void BeforeDestroy(Widgets::Widget*);

  [[nodiscard]]
  bool OnKeyPress(const KeyPressEvent& e);

  /// Thread-local
  static FocusManager* Get();
  static void PushInstance(FocusManager*);
  static void PopInstance(FocusManager*);

 private:
  Widgets::Widget* mRootWidget {};
  Widgets::Widget* mFocusedWidget {};
  FocusKind mFocusKind {FocusKind::Pointer};

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