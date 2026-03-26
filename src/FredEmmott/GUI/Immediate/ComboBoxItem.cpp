// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/Widgets/ComboBoxItemButton.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "PopupWindow.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {
using Widgets::ComboBoxItemButton;
}

ComboBoxItemResult<&EndComboBoxItem, void>
BeginComboBoxItem(bool* clicked, bool initiallySelected, const ID id) {
  using namespace immediate_detail;
  const auto item = BeginWidget<ComboBoxItemButton>(id);
  const bool activated = item->ConsumeWasActivated();
  if (activated) {
    ClosePopupWindow();
  }

  if (clicked) {
    *clicked = activated;
  }

  const bool isSelected = initiallySelected || activated;
  item->SetIsChecked(isSelected);

  BeginWidget<Widget>(
    ID {"content"}, LiteralStyleClass {"ComboBox/content"}, ImmutableStyle {});

  if (isSelected) {
    FUI_ASSERT(tWindow);
    tWindow->OffsetPositionToDescendant(GetCurrentParentNode());
  }

  return {item};
}

void EndComboBoxItem() {
  using namespace immediate_detail;
  EndWidget<Widget>();// content
  EndWidget<ComboBoxItemButton>();
}

ComboBoxItemResult<nullptr, bool>
ComboBoxItem(bool initiallySelected, std::string_view label, ID id) {
  bool clicked = false;
  const auto item = BeginComboBoxItem(&clicked, initiallySelected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return {item, clicked};
}

}// namespace FredEmmott::GUI::Immediate