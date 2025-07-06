// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/Windows/Win32Window.hpp>
#include <print>

#include "FredEmmott/GUI/Immediate/ContentDialog.hpp"

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

constexpr auto LoremIpsum
  = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. 💩 Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";

static void AppTick() {
  constexpr bool UseScrollView = true;
  if constexpr (UseScrollView) {
    fuii::BeginVScrollView().Styled({
      .mBackgroundColor
      = fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush,
    });
  }

  fuii::BeginVStackPanel().Styled({
    .mGap = 12,
    .mMargin = 12,
    .mPadding = 8,
  });

  fuii::Label("FUI Details").Subtitle();
  {
    // Could call EndCard() and EndVStackPanel() instead of .Scoped();
    const auto card = fuii::BeginCard().Scoped();
    const auto layout = fuii::BeginVStackPanel().Scoped();
    fuii::Label("Backend: {}", fui::GetBackendDescription());
    fuii::Label("_WIN32_WINNT: {:#010X}", _WIN32_WINNT);
    fuii::Label("NTDDI_VERSION: {:#010X}", NTDDI_VERSION);
  }

  fuii::Label("Controls").Subtitle();
  fuii::BeginCard();
  fuii::BeginVStackPanel();

  static bool sDisableAll = false;
  // (void) cast to ignore [[nodiscard]] is-changed return value
  fuii::ToggleSwitch(&sDisableAll).Caption("Disable all controls");
  fuii::BeginDisabled(sDisableAll);

  static bool sIsChecked {false};
  if (fuii::CheckBox(&sIsChecked, "I'm a checkbox!")) {
    std::println(
      stderr, "Checkbox changed to {}", sIsChecked ? "checked" : "unchecked");
  }

  fuii::Label("Hello, world; this text doesn't make the button wider aeg");
  static uint64_t frameCounter {};
  fuii::Label("Frame {}##Frames", ++frameCounter);

  static bool popupVisible = false;
  if (fuii::Button("Click Me!")
        .Caption("Button")
        .Styled({.mMinWidth = 200.f})) {
    popupVisible = true;
    std::println(stderr, "Clicked!");
  }

  if (const auto window = fuii::BeginPopup(&popupVisible).Scoped()) {
    const auto card = fuii::BeginCard().Scoped();
    const auto layout = fuii::BeginVStackPanel().Scoped();
    fuii::Label("This is a popup");
    if (fuii::Button("Close")) {
      popupVisible = false;
      fuii::EnqueueAdditionalFrame();
    }
  }

  static bool sShowAccentPopup = false;
  if (fuii::Button("Accent style").Caption("Accent button").Accent()) {
    std::println(stderr, "Accent clicked");
    sShowAccentPopup = true;
  }
  if (
    const auto dialog
    = fuii::BeginContentDialog(&sShowAccentPopup, "Test Popup").Scoped()) {
    fuii::Label("This is a ContentDialog.");
    fuii::TextBlock(LoremIpsum).Styled({.mMinWidth = 400});

    const auto footer = fuii::BeginContentDialogFooter().Scoped();
    if (fuii::Button("Save").Accent()) {
      std::println(stderr, "Save clicked");
      sShowAccentPopup = false;
    }
    if (fuii::Button("Close")) {
      std::println(stderr, "Close clicked");
      sShowAccentPopup = false;
    }
  }

  static bool isOn = true;
  if (fuii::ToggleSwitch(&isOn).Caption("ToggleSwitch")) {
    std::println(stderr, "Toggled to {}", isOn);
  }
  (void)(fuii::ToggleSwitch(&isOn)
           .Caption("ToggleSwitch with custom on/off text (discouraged)")
           .OnText("Foo (on)")
           .OffText("Bar (off)"));

  static int selectedIndex = 1;
  constexpr auto comboItems = std::array {
    "foo",
    "bar",
    "baz",
    "I am a much much much longer entry",
  };
  if (fuii::ComboBox(&selectedIndex, comboItems).Caption("ComboBox")) {
    std::println(stderr, "Combo changed to {}", comboItems[selectedIndex]);
  }

  fuii::BeginHStackPanel();
  // Glyphs areIUnicode private usage code points from
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  fuii::FontIcon("\ueb51");// Heart
  fuii::FontIcon("\ueb52");// HeartFill
  fuii::FontIcon({
    {"\ueb52", {.mColor = fui::Colors::Red}},
    {"\ueb51"},
  });
  fuii::Label("After stack");
  fuii::EndStackPanel();

  fuii::TextBlock(LoremIpsum);

  for (int i = 0; i < 5; ++i) {
    // IDs are automatically generated for formats
    fuii::Label("Formatted label {}", i);
  }
  for (int i = 0; i < 5; ++i) {
    // ... but not if we're passing in a variable like below,
    // so we need to explicitly manage state
    const auto scopedID = fuii::PushID(i).Scoped();
    fuii::Label(std::format("String label {}", i));
  }

  fuii::EndDisabled();

  fuii::EndStackPanel();
  fuii::EndCard();
  fuii::EndStackPanel();
  if constexpr (UseScrollView) {
    fuii::EndVScrollView();
  }
}

int WINAPI wWinMain(
  const HINSTANCE hInstance,
  const HINSTANCE hPrevInstance,
  const LPWSTR lpCmdLine,
  const int nCmdShow) {
  return fui::Win32Window::WinMain(
    hInstance,
    hPrevInstance,
    lpCmdLine,
    nCmdShow,
    [](fui::Win32Window&) { AppTick(); },
    {"FUI Demo"});
}
