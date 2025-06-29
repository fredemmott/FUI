// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemTheme.hpp"

#include <Windows.h>
#include <wil/com.h>
#include <wil/winrt.h>
#include <windows.ui.viewmanagement.h>
#include <winuser.h>

#include "Color.hpp"
#include "detail/win32_detail.hpp"

#define FUI_SYSTEM_COLOR_USAGES(X) \
  FUI_WINAPI_SYS_COLORS(X) \
  FUI_WINRT_UI_ACCENT_COLORS(X)

using namespace FredEmmott::GUI::win32_detail;
using namespace ABI::Windows::UI::ViewManagement;

namespace FredEmmott::GUI::SystemTheme {
struct Store {
  Store();
  void Populate();
  void Populate(IUISettings3&, Color::Constant*, UIColorType) const;
  void Populate(IUISettings3&, Color::Constant*, int) const;

  static Store& Get() {
    static Store ret;
    return ret;
  }

#define DECLARE_USAGE_VARIABLE(X, IMPL) Color::Constant m##X {};
  FUI_SYSTEM_COLOR_USAGES(DECLARE_USAGE_VARIABLE)
#undef DECLARE_USAGE_VARIABLE
};

Store::Store() {
  this->Populate();
}

void Store::Populate() {
  const wil::com_ptr<IUISettings3> uiSettings
    = wil::ActivateInstance<IUISettings3>(
      L"Windows.UI.ViewManagement.UISettings");
#define POPULATE_COLOR(X, IMPL) this->Populate(*uiSettings, &m##X, IMPL);
  FUI_SYSTEM_COLOR_USAGES(POPULATE_COLOR)
#undef POPULATE_COLOR
}

void Store::Populate(
  IUISettings3& uiSettings,
  Color::Constant* p,
  const UIColorType type) const {
  ABI::Windows::UI::Color ret;
  CheckHResult(uiSettings.GetColorValue(type, &ret));
  const auto [a, r, g, b] = ret;
  *p = Color::Constant::FromARGB32(a, r, g, b);
}

void Store::Populate(IUISettings3&, Color::Constant* p, int sysColor) const {
  const auto color = GetSysColor(sysColor);
  *p = Color::Constant::FromARGB32(
    0xff, GetRValue(color), GetGValue(color), GetBValue(color));
}

Color Resolve(const ColorType usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X, IMPL) \
  case ColorType::X: \
    return Store::Get().m##X;
    FUI_SYSTEM_COLOR_USAGES(USAGE_CASE);
#undef USAGE_CASE
  }
  std::unreachable();
}

void Refresh() {
  Store::Get() = {};
  Store::Get().Populate();
}

}// namespace FredEmmott::GUI::SystemTheme