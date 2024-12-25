// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemTheme.hpp"

#include <Windows.h>
#include <winrt/windows.ui.viewmanagement.h>
#include <winuser.h>

#include "Color.hpp"

#define FUI_SYSTEM_COLOR_USAGES(X) \
  FUI_WINAPI_SYS_COLORS(X) \
  FUI_WINRT_UI_ACCENT_COLORS(X)

using namespace winrt::Windows::UI::ViewManagement;

namespace FredEmmott::GUI::SystemTheme {
// TODO: automatically update when user changes theme, or light <-> dark
struct Store {
  Store();
  void Populate();
  void Populate(SkColor*, UIColorType) const;
  void Populate(SkColor*, int) const;

#define DECLARE_USAGE_VARIABLE(X, IMPL) SkColor m##X {};
  FUI_SYSTEM_COLOR_USAGES(DECLARE_USAGE_VARIABLE)
#undef DECLARE_USAGE_VARIABLE

 private:
  UISettings mUISettings;
};

Store::Store() {
  this->Populate();
}

void Store::Populate() {
  using enum UIColorType;
#define POPULATE_COLOR(X, IMPL) this->Populate(&m##X, IMPL);
  FUI_SYSTEM_COLOR_USAGES(POPULATE_COLOR)
#undef POPULATE_COLOR
}

void Store::Populate(SkColor* p, const UIColorType type) const {
  auto [a, r, g, b] = mUISettings.GetColorValue(type);
  *p = SkColorSetARGB(a, r, g, b);
}

void Store::Populate(SkColor* p, int sysColor) const {
  const auto color = GetSysColor(sysColor);
  *p = SkColorSetRGB(GetRValue(color), GetGValue(color), GetBValue(color));
}

static Store gStore;

Color Resolve(const ColorType usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X, IMPL) \
  case ColorType::X: \
    return gStore.m##X;
    FUI_SYSTEM_COLOR_USAGES(USAGE_CASE);
#undef USAGE_CASE
  }
  std::unreachable();
}

void Refresh() {
  gStore = {};
  gStore.Populate();
}

}// namespace FredEmmott::GUI::SystemTheme