// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "demo.hpp"

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <map>
#include <print>

constexpr auto LoremIpsum
  = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. 💩 Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";

fuii::VStackPanelResult BeginDemoPage() {
  return fuii::BeginVStackPanel().Styled(
    fui::Style().FlexGrow(1).Gap(12).Margin(12).Padding(8));
}

fuii::CardResult BeginDemoCard(const fuii::ID id) {
  return fuii::BeginCard(id).Styled(
    fui::Style()
      .FlexDirection(YGFlexDirectionColumn)
      .Gap(12)
      .Margin(12)
      .Margin(8));
}

static void demo_display() {
  const auto scroll = fuii::BeginVScrollView().Scoped();
  const auto page = BeginDemoPage().Scoped();

  {
    fuii::Label("TextBlock").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    fuii::Label("Label()");
    fuii::Label("Label().Caption()").Caption();
    fuii::Label("Label().Body()").Body();
    fuii::Label("Label().BodyStrong()").BodyStrong();
    fuii::Label("Label().Subtitle()").Subtitle();
    fuii::Label("Label().Title()").Title();
    fuii::Label("Label().TitleLarge()").TitleLarge();
    fuii::Label("Label().Display()").Display();
  }

  {
    fuii::Label("FontIcon").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    // Glyphs are Unicode private usage code points from
    // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
    fuii::FontIcon("\ueb51").Caption("FontIcon(Heart)");
    fuii::FontIcon("\ueb52").Caption("FontIcon(HeartFill)");
    fuii::FontIcon({
                     {"\ueb52", fui::Style().Color(fui::Colors::Red)},
                     {"\ueb51"},
                   })
      .Caption("FontIcon({HeartFill, Heart})");
  }

  {
    fuii::Label("TextBlock").Subtitle();
    const auto card = BeginDemoCard().Scoped();
    fuii::TextBlock(LoremIpsum);
  }

  {
    fuii::Label("ProgressRing").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static bool isActive = false;
    std::ignore = fuii::ToggleSwitch(&isActive).Caption("ProgressRing()");
    fuii::ProgressRing().Active(isActive);

    static float value = 0.0f;
    fuii::HSlider(&value)
      .Caption("ProgressRing(value)")
      .TickFrequency(20)
      .ValueFormatter([](const float f) { return std::format("{:.0f}%", f); });
    fuii::ProgressRing(value);
  }
}

void demo_buttons() {
  const auto scroll = fuii::BeginVScrollView().Scoped();
  const auto page = BeginDemoPage().Scoped();
  const auto card = BeginDemoCard().Scoped();

  if (fuii::Button("Button()")) {
    std::println(stderr, "Button clicked");
  }

  if (fuii::Button("Button().Accent()").Accent()) {
    std::println(stderr, "Accent button clicked");
  }
}

static void demo_booleans() {
  const auto scroll = fuii::BeginVScrollView().Scoped();
  const auto page = BeginDemoPage().Scoped();

  {
    const auto card = BeginDemoCard().Scoped();

    static bool checked {false};
    if (fuii::CheckBox(&checked, "CheckBox()").Caption("CheckBox()")) {
      std::println(stderr, "Checked to {}", checked);
    }

    static bool toggled {false};
    if (fuii::ToggleSwitch(&toggled).Caption("ToggleSwitch()")) {
      std::println(stderr, "Toggled to {}", toggled);
    }
  }

  {
    fuii::Label("Discouraged").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    fuii::Label(
      "Microsoft style guidelines discourage custom on/off text; use "
      ".Caption(...) instead");

    static bool toggledWithCustomText {false};
    if (fuii::ToggleSwitch(&toggledWithCustomText)
          .OffText("Disabled")
          .OnText("Enabled")
          .Caption(
            "ToggleSwitch().OffText(\"Disabled\").OnText(\"Enabled\")")) {
      std::println(stderr, "Toggled to {}", toggledWithCustomText);
    }
  }
}

static void demo_selection() {
  const auto scroll = fuii::BeginVScrollView().Scoped();
  const auto page = BeginDemoPage().Scoped();

  {
    fuii::Label("Radio Buttons").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static std::size_t selected = 1;
    const auto buttons
      = fuii::BeginRadioButtons(&selected, "Group Header").Scoped();
    for (std::size_t i = 0; i < 3; ++i) {
      if (fuii::RadioButton(i, "Option {}", i)) {
        std::println(stderr, "Radio changed to {}", i);
      }
    }
  }

  {
    fuii::Label("Combo Boxes").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    {
      static int selectedIndex = 1;
      constexpr auto comboItems = std::array {
        "foo",
        "bar",
        "baz",
      };
      if (fuii::ComboBox(&selectedIndex, comboItems)
            .Caption("Array of strings")) {
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
      if (fuii::ComboBox(&selectedIndex, comboItems, &Item::mLabel)
            .Caption("Array of structs")) {
        std::println(
          stderr, "Combo changed to {}", comboItems[selectedIndex].mLabel);
      }
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
  }
}

static void demo_input() {
  const auto scroll = fuii::BeginVScrollView().Scoped();
  const auto page = BeginDemoPage().Scoped();

  {
    fuii::Label("Text Box").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static std::string textBoxValue {"Hello, 💩 world!"};
    if (fuii::TextBox(&textBoxValue).Caption("TextBox()")) {
      std::println(stderr, "TextBox value is {}", textBoxValue);
    }
  }

  {
    fuii::Label("Number Box").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static std::optional<int> numberBoxValue;
    if (fuii::NumberBox(&numberBoxValue).Caption("NumberBox()")) {
      if (numberBoxValue) {
        std::println(stderr, "NumberBox value is {}", numberBoxValue.value());
      } else {
        std::println(stderr, "NumberBox has no value");
      }
    }
  }

  {
    fuii::Label("Sliders").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static float hslider {};
    if (fuii::HSlider(&hslider).TickFrequency(25).Caption(
          "HSlider().TickFrequency(25)")) {
      std::println(stderr, "Slider value is {}", hslider);
    }

    static float vslider {};
    if (fuii::VSlider(&vslider)
          .TickFrequency(25)
          .SnapToTicks()
          .Styled(fui::Style().Height(120))
          .Caption("VSlider().TickFrequency(25).SnapToTicks()")) {
      std::println(stderr, "Slider value is {}", vslider);
    }
  }
}

static void demo_popups() {
  const auto scroll = fuii::BeginVScrollView().Scoped();
  const auto page = BeginDemoPage().Scoped();

  {
    fuii::Label("Basic Popup").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static bool visible = false;
    if (fuii::Button("Click Me!")) {
      visible = true;
      std::println(stderr, "Clicked!");
    }
    if (const auto popup = fuii::BeginPopup(&visible).Scoped()) {
      const auto popupCard = BeginDemoCard().Scoped();

      fuii::Label("This is a popup");
      if (fuii::Button("Close")) {
        fuii::ClosePopupWindow();
      }
    }
  }

  {
    fuii::Label("Content Dialog").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    static bool visible = false;
    if (fuii::Button("Click Me!")) {
      visible = true;
      std::println(stderr, "Clicked!");
    }

    if (const auto dialog = fuii::BeginContentDialog(&visible).Scoped()) {
      fuii::ContentDialogTitle("Demo dialog title");
      fuii::Label("This is a ContentDialog.");

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
  }

  {
    fuii::Label("Tooltips").Subtitle();
    const auto card = BeginDemoCard().Scoped();

    std::ignore = fuii::Button("Hover here")
                    .ToolTip("Tooltip")
                    .Caption("Button().Tooltip()");

    std::ignore = fuii::Button("Hover here")
                    .Caption("Button(); BeginTooltipForPreviousWidget();");
    if (auto tt = fuii::BeginToolTipForPreviousWidget().Scoped()) {
      fuii::Label("This is a tooltip");
    }
  }
}

static void demo_about() {
  const auto page = BeginDemoPage().Scoped();
  const auto card = BeginDemoCard().Scoped();

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

static void AppTick(fui::Window&) {
  fuii::WindowTitle("FUI Demo");
  std::ignore = fuii::WindowSubtitle("I like turtles");

  enum class Page {
    Display,
    Buttons,
    Booleans,
    Selections,
    Input,
    Popups,
#ifdef _WIN32
    Win32,
#endif
    About,
  };
  static Page sCurrentPage {Page::Display};

  const auto nav
    = fuii::BeginNavigationView(&sCurrentPage).IntegrateWithTitleBar().Scoped();

  if (
    const auto page
    = fuii::BeginNavigationViewItem(Page::Display, "\ue7f4", "Display")
        .Scoped()) {
    demo_display();
  }

  if (
    const auto page
    = fuii::BeginNavigationViewItem(Page::Buttons, "\uf8af", "Buttons")
        .Scoped()) {
    demo_buttons();
  }

  if (
    const auto page
    = fuii::BeginNavigationViewItem(Page::Booleans, "\ue762", "Booleans")
        .Scoped()) {
    demo_booleans();
  }

  if (
    const auto page
    = fuii::BeginNavigationViewItem(Page::Selections, "\ueccb", "Selections")
        .Scoped()) {
    demo_selection();
  }

  if (
    const auto page
    = fuii::BeginNavigationViewItem(Page::Input, "\ue961", "Input").Scoped()) {
    demo_input();
  }

  if (
    const auto page
    = fuii::BeginNavigationViewItem(Page::Popups, "\ueb91", "Popups")
        .Scoped()) {
    demo_popups();
  }

#ifdef _WIN32
  // Glyph is "OEM"
  if (
    const auto navItem
    = fuii::BeginNavigationViewItem(Page::Win32, "\ue74c", "Win32").Scoped()) {
    demo_win32();
  }
#endif

  const auto navFooter = fuii::BeginNavigationViewFooterItems().Scoped();
  if (
    const auto page
    = fuii::BeginNavigationViewSettingsItem(Page::About, "About", "About")
        .Scoped()) {
    demo_about();
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
    {
      "Initial Window Title",
      fui::Window::ResizeMode::AllowGrow,
      fui::Window::ResizeMode::Allow,
    });
}
