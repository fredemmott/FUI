// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/Immediate/Button.hpp>
#include <FredEmmott/GUI/Immediate/Card.hpp>
#include <FredEmmott/GUI/Immediate/Disabled.hpp>
#include <FredEmmott/GUI/Immediate/FontIcon.hpp>
#include <FredEmmott/GUI/Immediate/Label.hpp>
#include <FredEmmott/GUI/Immediate/StackPanel.hpp>
#include <FredEmmott/GUI/Immediate/ToggleSwitch.hpp>
#include <print>

#include "Win32-Ganesh-D3D12.hpp"

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

static void AppTick() {
  fuii::BeginCard();
  fuii::BeginVStackPanel();
  fuii::Label("Disable all widgets");
  static bool sDisableAll = false;
  (void)fuii::ToggleSwitch(&sDisableAll);
  fuii::BeginDisabled(sDisableAll);

  fuii::Label("Hello, world; this text doesn't make the button wider aeg");
  static uint64_t frameCounter {};
  fuii::Label("Frame {}##Frames", ++frameCounter);
  if (fuii::Button("Click Me!")) {
    std::println(stderr, "Clicked!");
  }

  static bool isOn = true;
  if (fuii::ToggleSwitch(&isOn)) {
    std::println(stderr, "Toggled to {}", isOn);
  }

  fuii::EndDisabled();

  fuii::BeginHStackPanel();
  fuii::FontIcon("\ueb51");// Heart
  fuii::FontIcon("\ueb52");// HeartFill
  fuii::FontIcon({
    {"\ueb52", {{.mColor = SK_ColorRED}}},
    {"\ueb51"},
  });
  fuii::Label("After stack");
  fuii::EndStackPanel();

  fuii::EndStackPanel();
  fuii::EndCard();
}

int WINAPI wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR lpCmdLine,
  int nCmdShow) {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  HelloSkiaWindow window(hInstance);
  ShowWindow(window.GetHWND(), nCmdShow);
  while (true) {
    const auto ok = window.BeginFrame();
    if (!ok) {
      return ok.error();
    }

    AppTick();

    window.EndFrame();
    window.WaitFrame();
  }
}
