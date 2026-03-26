// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "MenuFlyout.hpp"

#include "FredEmmott/GUI/StaticTheme/MenuFlyout.hpp"
#include "FredEmmott/GUI/Widgets/MenuFlyoutItem.hpp"
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "PopupWindow.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {

constexpr LiteralStyleClass MenuFlyoutStyleClass {"MenuFlyout"};
constexpr LiteralStyleClass MenuFlyoutSeparatorStyleClass {
  "MenuFlyout/Separator"};

}// namespace

MenuFlyoutResult BeginMenuFlyout(const ID id) {
  if (!BeginBasicPopupWindow(id)) {
    return false;
  }

  immediate_detail::BeginWidget<Widgets::Widget>(
    ID {0}, MenuFlyoutStyleClass, StaticTheme::MenuFlyout::MenuFlyoutStyle());

  return true;
}

void EndMenuFlyout() {
  immediate_detail::EndWidget();
  const auto win32 = static_cast<Win32Window*>(immediate_detail::tWindow);

  EndBasicPopupWindow();

  const auto hwnd = win32->GetNativeHandle().mValue;
  const MARGINS margins {-1};
  DwmExtendFrameIntoClientArea(hwnd, &margins);
}

MenuFlyoutResult BeginMenuFlyout(bool* const open, const ID id) {
  FUI_ASSERT(open);
  if (!*open) {
    return false;
  }
  *open = BeginMenuFlyout(id);
  return *open;
}

MenuFlyoutItemResult MenuFlyoutItem(
  const std::string_view glyph,
  const std::string_view label,
  const ID id) {
  const auto w = immediate_detail::ChildlessWidget<Widgets::MenuFlyoutItem>(id);
  w->SetGlyph(glyph);
  w->SetLabel(label);
  const auto activated = w->ConsumeWasActivated();
  if (activated) {
    ClosePopupWindow();
  }
  return {w, activated};
}

void MenuFlyoutSeparator(const ID id) {
  immediate_detail::ChildlessWidget<Widgets::Widget>(
    id,
    MenuFlyoutSeparatorStyleClass,
    StaticTheme::MenuFlyout::MenuFlyoutSeparatorStyle());
}

}// namespace FredEmmott::GUI::Immediate
