// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <winrt/windows.ui.viewmanagement.h>

#include <magic_enum.hpp>
#include <magic_enum_utility.hpp>

#include "SystemColor.hpp"

using namespace winrt::Windows::UI::ViewManagement;

namespace FredEmmott::GUI {
SystemColor::SystemColor() {
  this->Populate();
}

const SystemColor& SystemColor::Get() {
  static SystemColor sInstance;
  return sInstance;
}

void SystemColor::Populate() {
  this->mColors.clear();
  magic_enum::enum_for_each<Usage>(
    [&colors = this->mColors, store = UISettings {}](const Usage key) {
      const auto name = magic_enum::enum_name(key);
      const auto winKey = magic_enum::enum_cast<UIColorType>(name);
      if (!winKey.has_value()) {
        return;
      }
      const auto [a, r, g, b] = store.GetColorValue(*winKey);
      colors.emplace(key, SkColorSetARGB(a, r, g, b));
    });
}
}// namespace FredEmmott::GUI