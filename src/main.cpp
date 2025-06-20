// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/Widgets/ScrollBar.hpp>
#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <print>

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

static void AppTick() {
  constexpr bool UseScrollView = true;
  if constexpr (UseScrollView) {
    fuii::immediate_detail::BeginWidget<fui::Widgets::ScrollView>(
      fuii::ID {"scrollView"});
    auto sv = fuii::immediate_detail::GetCurrentParentNode<
      fui::Widgets::ScrollView>();
    sv->SetVerticalScrollBarVisibility(
      fui::Widgets::ScrollView::ScrollBarVisibility::Auto);
  }

  fuii::BeginCard();
  fuii::BeginVStackPanel();
  fuii::Label("Disable all widgets");
  static bool sDisableAll = false;
  // (void) cast to ignore [[nodiscard]] is-changed return value
  (void)fuii::ToggleSwitch(&sDisableAll);
  fuii::BeginDisabled(sDisableAll);

  fuii::Label("Hello, world; this text doesn't make the button wider aeg");
  static uint64_t frameCounter {};
  fuii::Label("Frame {}##Frames", ++frameCounter);

  static bool popupVisible = false;
  if (fuii::Button("Click Me!")) {
    popupVisible = true;
    std::println(stderr, "Clicked!");
  }

  if (fuii::BeginPopupWindow(&popupVisible)) {
    fuii::BeginCard();
    fuii::BeginVStackPanel();
    fuii::Label("This is a popup");
    if (fuii::Button("Close")) {
      popupVisible = false;
      fuii::EnqueueAdditionalFrame();
    }
    fuii::EndStackPanel();
    fuii::EndCard();
    fuii::EndPopupWindow();
  }

  static bool isOn = true;
  if (fuii::ToggleSwitch(&isOn)) {
    std::println(stderr, "Toggled to {}", isOn);
  }

  static bool comboBoxVisible = false;
  static int selectedIndex = 1;
  constexpr auto comboItems = std::array {
    "foo",
    "bar",
    "baz",
  };
  if (fuii::ComboBox(&selectedIndex, comboItems)) {
    std::println(stderr, "Combo changed to {}", comboItems[selectedIndex]);
  }

  fuii::BeginHStackPanel();
  // Glyphs areIUnicode private usage code points from
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  fuii::FontIcon("\ueb51");// Heart
  fuii::FontIcon("\ueb52");// HeartFill
  fuii::FontIcon({
    {"\ueb52", {.mColor = SK_ColorRED}},
    {"\ueb51"},
  });
  fuii::Label("After stack");
  fuii::EndStackPanel();

  fuii::TextBlock(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. 💩 Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.");
  fuii::EndDisabled();

  fuii::EndStackPanel();
  fuii::EndCard();
  if constexpr (UseScrollView) {
    fuii::immediate_detail::EndWidget<fui::Widgets::ScrollView>();
  }
}

int WINAPI wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR lpCmdLine,
  int nCmdShow) {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  fui::Window window(hInstance, nCmdShow, {"FUI Demo"});
  while (true) {
    // Variable FPS - wait for whichever is sooner:
    // - input
    // - target frame interval
    //
    // The default target FPS varies; it is '0 fps - input only' usually, but
    // is 60FPS when an animation is active.
    window.WaitFrame();

    const auto ok = window.BeginFrame();
    if (!ok) {
      return ok.error();
    }

    AppTick();

    window.EndFrame();
  }
}
