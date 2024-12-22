// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemColor.hpp"

#include <winrt/windows.ui.viewmanagement.h>

#include "Color.hpp"

#define FUI_SYSTEM_COLOR_USAGES(X) \
  X(Background) \
  X(Foreground) \
  X(AccentDark3) \
  X(AccentDark2) \
  X(AccentDark1) \
  X(Accent) \
  X(AccentLight1) \
  X(AccentLight2) \
  X(AccentLight3)

using namespace winrt::Windows::UI::ViewManagement;

namespace FredEmmott::GUI::SystemColor {
// TODO: automatically update when user changes theme, or light <-> dark
struct Store {
  Store();
  void Populate();
  void Populate(SkColor*, UIColorType) const;

#define DECLARE_USAGE_VARIABLE(X) SkColor m##X {};
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
#define POPULATE_COLOR(X) this->Populate(&m##X, X);
  FUI_SYSTEM_COLOR_USAGES(POPULATE_COLOR);
#undef POPULATE_COLOR
}

void Store::Populate(SkColor* p, const UIColorType type) const {
  auto [a, r, g, b] = mUISettings.GetColorValue(type);
  *p = SkColorSetARGB(a, r, g, b);
}

static Store gStore;

Color Resolve(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X) \
  case Usage::X: \
    return gStore.m##X;
    FUI_SYSTEM_COLOR_USAGES(USAGE_CASE);
#undef USAGE_CASE
  }
  std::unreachable();
}

#define DEFINE_CONVENIENCE_ACCESSOR(X) \
  Color X() noexcept { \
    return { Usage::X }; \
  }
FUI_SYSTEM_COLOR_USAGES(DEFINE_CONVENIENCE_ACCESSOR)
#undef DEFINE_CONVENIENCE_ACCESSOR

}// namespace FredEmmott::GUI::SystemColor