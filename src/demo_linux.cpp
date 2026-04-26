// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux demo entry. When ENABLE_SKIA is on this runs the real FUI frame
// loop against LinuxSkiaVulkanWindow (Skia Ganesh on Vulkan). When
// ENABLE_SKIA is off it falls back to LinuxWindow::Run() .

#include <cstdio>

#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/Linux/LinuxWindow.hpp>

#include "demo.hpp"

#ifdef FUI_ENABLE_SKIA
#include <FredEmmott/GUI/Linux/LinuxSkiaVulkanWindow.hpp>
#endif

namespace fui = FredEmmott::GUI;

int main(int, char**) {
#ifdef FUI_ENABLE_SKIA
  std::fprintf(
    stderr,
    "FUI Linux demo (Skia + Vulkan). Esc / window-close quits.\n");
  return fui::LinuxSkiaVulkanWindow::Run(
    {
      .mTitle = "FUI Demo (Linux / Skia / Vulkan)",
      .mInitialSize = {960, 640},
    },
    [](fui::LinuxSkiaVulkanWindow& window) { AppTick(window); });
#else
  std::fprintf(
    stderr,
    "FUI Linux demo. Esc quits.\n");
  return fui::LinuxWindow::Run(
    {.mTitle = "FUI Linux demo"},
    [](fui::LinuxWindow&) {});
#endif
}
