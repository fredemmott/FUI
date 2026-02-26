// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <atomic>
#include <map>
#include <print>

#ifdef _WIN32
#include "demo_win32.hpp"
#endif

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

static void AppTick(fui::Window& window) {
  constexpr bool UseScrollView = true;
  if constexpr (UseScrollView) {
    using enum fui::Window::ResizeMode;
    window.SetResizeMode(AllowGrow, AllowShrink);
    fuii::BeginVScrollView().Styled(
      fui::Style()
        .BackgroundColor(
          fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush)
        .FlexGrow(1));
  }

  fuii::BeginVStackPanel().Styled(
    fui::Style().FlexGrow(1).Gap(12).Margin(12).Padding(8));

  fuii::Label("FUI Details").Subtitle();
  {
    // Could call EndCard() and EndVStackPanel() instead of .Scoped();
    const auto card = fuii::BeginCard().Scoped();
    const auto layout = fuii::BeginVStackPanel().Scoped();
    fuii::Label(
      "Backend: {} ({} ICU)",
      fui::GetBackendDescription(),
#ifdef FUI_ENABLE_ICU
      "bundled"
#else
      "Windows"
#endif
    );
    fuii::Label("_WIN32_WINNT: {:#010X}", _WIN32_WINNT);
    fuii::Label("NTDDI_VERSION: {:#010X}", NTDDI_VERSION);
  }

  fuii::Label("Controls").Subtitle();
  fuii::BeginCard();
  fuii::BeginVStackPanel().Styled(fui::Style().FlexGrow(1));

  static bool sDisableAll = false;
  // (void) cast to ignore [[nodiscard]] is-changed return value
  fuii::ToggleSwitch(&sDisableAll).Caption("Disable all controls");
  fuii::BeginDisabled(sDisableAll);

  static bool sIsChecked {false};
  if (fuii::CheckBox(&sIsChecked, "I'm a checkbox!")
        .ToolTip("CheckBox ToolTip")) {
    std::println(
      stderr, "Checkbox changed to {}", sIsChecked ? "checked" : "unchecked");
  }

  fuii::Label("Hello, world; this text doesn't make the button wider aeg");

  static bool popupVisible = false;
  if (fuii::Button("Click Me!")
        .Caption("Button")
        .Styled(fui::Style().MinWidth(200.f))) {
    popupVisible = true;
    std::println(stderr, "Clicked!");
  }
  if (auto tt = fuii::BeginToolTipForPreviousWidget().Scoped()) {
    fuii::Label("This is a tooltip");
  }

  if (const auto popup = fuii::BeginPopup(&popupVisible).Scoped()) {
    const auto card = fuii::BeginCard().Scoped();
    const auto layout = fuii::BeginVStackPanel().Scoped();
    fuii::Label("This is a popup");
    if (fuii::Button("Close")) {
      popupVisible = false;
    }
  }

  static bool sShowAccentPopup = false;
  if (fuii::Button("Accent style")
        .Caption("Accent button")
        .Accent()
        .ToolTip("Fluent ToolTip")) {
    std::println(stderr, "Accent clicked");
    sShowAccentPopup = true;
  }
  if (
    const auto dialog = fuii::BeginContentDialog(&sShowAccentPopup).Scoped()) {
    fuii::ContentDialogTitle("Demo dialog title");
    fuii::Label("This is a ContentDialog.");
    fuii::TextBlock(LoremIpsum).Styled(fui::Style().MinWidth(400));

    const auto footer = fuii::BeginContentDialogButtons().Scoped();
    if (fuii::ContentDialogPrimaryButton("Test").Accent()) {
      std::println(stderr, "ContentDialog primary button clicked");
    }
    fuii::BeginDisabled();
    fuii::ContentDialogSecondaryButton("Test Disabled");
    fuii::EndDisabled();
    if (fuii::ContentDialogCloseButton()) {
      std::println(stderr, "ContentDialog close button clicked");
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

  {
    static int selectedIndex = 1;
    constexpr auto comboItems = std::array {
      "foo",
      "bar",
      "baz",
    };
    if (fuii::ComboBox(&selectedIndex, comboItems)
          .Caption("Array of strings")
          .ToolTip("CB ToolTip")) {
      std::println(stderr, "Combo changed to {}", comboItems[selectedIndex]);
    }
  }
  {
    struct Item {
      std::string_view mLabel;
    };
    static int selectedIndex = 1;
    constexpr auto comboItems = std::array {
      Item {"a"},
      Item {"b"},
      Item {"c"},
    };
    (void)fuii::ComboBox(&selectedIndex, comboItems, &Item::mLabel)
      .Caption("Array of structs");
  }
  {
    struct Item {
      int mKey;
      std::string_view mLabel;
    };
    static int selectedIndex = 1;
    constexpr auto comboItems = std::array {
      Item {123, "a"},
      Item {456, "b"},
      Item {789, "c"},
    };
    if (fuii::ComboBox(&selectedIndex, comboItems, &Item::mLabel, &Item::mKey)
          .Caption("Key-value structs")) {
      std::println(stderr, "value is {}", selectedIndex);
    }
  }
  {
    static int selectedIndex = 1;
    const auto comboItems = std::map<int, std::string_view> {
      {123, "Foo"},
      {456, "Bar"},
      {789, "echo echo echo"},
    };
    if (fuii::ComboBox(&selectedIndex, comboItems).Caption("std::map")) {
      std::println(stderr, "value is {}", selectedIndex);
    }
  }

  static std::size_t selectedOption = 1;
  fuii::BeginRadioButtons("Radio Buttons Header");
  for (std::size_t i = 0; i < 3; ++i) {
    if (fuii::RadioButton(&selectedOption, i, "Option {}", i)) {
      std::println(stderr, "Radio changed to {}", i);
    }
  }
  fuii::EndRadioButtons();

  fuii::BeginHStackPanel();
  // Glyphs areIUnicode private usage code points from
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  fuii::FontIcon("\ueb51");// Heart
  fuii::FontIcon("\ueb52");// HeartFill
  fuii::FontIcon({
    {"\ueb52", fui::Style().Color(fui::Colors::Red)},
    {"\ueb51"},
  });
  fuii::Label("After stack");
  fuii::EndStackPanel();

  fuii::TextBlock(LoremIpsum);

  for (int i = 0; i < 3; ++i) {
    // IDs are automatically generated for formats
    fuii::Label("Formatted label {}", i);
  }
  for (int i = 0; i < 3; ++i) {
    // ... but not if we're passing in a variable like below,
    // so we need to explicitly manage state
    const auto scopedID = fuii::PushID(i).Scoped();
    fuii::Label(std::format("String label {}", i));
  }

  if (fuii::HyperlinkButton("I'm a HyperlinkButton!")) {
    std::println(stderr, "Hyperlink clicked");
  }

  static std::string text {"Hello 💩 world"};
  if (fuii::TextBox(&text).Caption("I'm a TextBox!")) {
    std::println(stderr, "TextBox changed to {}", text);
  }

  static float sliderValue {0};
  fuii::HSlider(&sliderValue).TickFrequency(25);
  fuii::VSlider(&sliderValue)
    .TickFrequency(25)
    .SnapToTicks()
    .Styled(fui::Style().Height(120));

  static std::optional<int> numberBoxValue;
  fuii::NumberBox(&numberBoxValue);

#ifdef _WIN32
  demo_win32();
#endif

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
    [](fui::Win32Window& window) { AppTick(window); },
    {"FUI Demo"});
}
