// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/Windows/Win32Window.hpp>
#include <print>

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

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

  fuii::Label("Disable all controls").Caption();
  static bool sDisableAll = false;
  // (void) cast to ignore [[nodiscard]] is-changed return value
  (void)fuii::ToggleSwitch(&sDisableAll);
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
  if (fuii::Button("Click Me!")) {
    popupVisible = true;
    std::println(stderr, "Clicked!");
  }

  if (fuii::Button("Accent style").Accent()) {
    std::println(stderr, "Accent clicked");
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

  static int selectedIndex = 1;
  constexpr auto comboItems = std::array {
    "foo",
    "bar",
    "baz",
    "I am a much much much longer entry",
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
    {"\ueb52", {.mColor = fui::Colors::Red}},
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
