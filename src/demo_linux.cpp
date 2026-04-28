// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux demo entry. Runs the FUI frame loop against SdlSkiaVulkanWindow
// (Skia Ganesh on Vulkan).

#include <cstdio>

#include <FredEmmott/GUI/Sdl/SdlSkiaVulkanWindow.hpp>

#include "demo.hpp"

namespace fui = FredEmmott::GUI;

int main(int, char**) {
  std::fprintf(
    stderr, "FUI Linux demo (Skia + Vulkan). Esc / window-close quits.\n");
  return fui::SdlSkiaVulkanWindow::Run(
    {
      .mTitle = "FUI Demo (Linux / Skia / Vulkan)",
      .mInitialSize = {960, 640},
    },
    [](fui::SdlSkiaVulkanWindow& window) { AppTick(window); });
}
