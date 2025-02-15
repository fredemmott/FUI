// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/detail/ComboBoxStyles.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginComboBoxItem(bool* clicked, bool initiallySelected, ID id) {
  using namespace immediate_detail;
  BeginButton(clicked, id);
  const bool isSelected = initiallySelected || (clicked && *clicked);
  if (isSelected) {
    FUI_ASSERT(tWindow);
    tWindow->OffsetPositionToDescendant(GetCurrentParentNode());
  }

  const auto item = GetCurrentParentNode();
  item->AddStyleClass(ComboBoxItemStyleClass());
  item->AppendBuiltInStyleSheet(ComboBoxItemStyles());
  item->ToggleStyleClass(PseudoClasses::Checked, isSelected);
  BeginHStackPanel();
  GetCurrentParentNode()->AppendBuiltInStyles({.mGap = 0.0});
  BeginWidget<Widget>(ID {"pill"});
  auto pill = GetCurrentParentNode();
  pill->AddStyleClass(ComboBoxItemPillStyleClass());
  pill->AppendBuiltInStyleSheet(ComboBoxItemPillStyles());
  EndWidget<Widget>();
  BeginWidget<Widget>(ID {"content"});
  GetCurrentParentNode()->AppendBuiltInStyles({
    .mDisplay = YGDisplayContents,
  });
}

void EndComboBoxItem() {
  using namespace immediate_detail;
  EndWidget<Widget>();// content
  EndStackPanel();
  EndButton();
}

bool ComboBoxItem(bool initiallySelected, std::string_view label, ID id) {
  bool clicked = false;
  BeginComboBoxItem(&clicked, initiallySelected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return clicked;
}

}// namespace FredEmmott::GUI::Immediate