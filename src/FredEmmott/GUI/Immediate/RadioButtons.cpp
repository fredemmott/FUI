// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "RadioButtons.hpp"

#include <FredEmmott/GUI/StaticTheme/RadioButtons.hpp>

#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {
void EndRadioButtons() {
  EndVStackPanel();
  EndVStackPanel();
}
Result<&EndRadioButtons> BeginRadioButtons(
  const std::string_view title,
  const ID id) {
  using namespace StaticTheme::RadioButtons;
  using namespace PseudoClasses;

  const auto outer = BeginVStackPanel(id).Styled(
    Style().FlexDirection(YGFlexDirectionColumn).Gap(0));

  if (!title.empty()) {
    static const auto TitleStyle
      = Style()
          .Color(RadioButtonsHeaderForeground)
          .MarginBottom(RadioButtonsTopHeaderMarginBottom)
          .MarginLeft(RadioButtonsTopHeaderMarginLeft)
          .MarginRight(RadioButtonsTopHeaderMarginRight)
          .MarginTop(RadioButtonsTopHeaderMarginTop)
          .And(Disabled, Style().Color(RadioButtonsHeaderForegroundDisabled));
    Label(title, ID {"RadioButtons/Title"}).Styled(TitleStyle);
  }

  BeginVStackPanel(ID {"RadioButtons/Content"})
    .Styled(Style().Gap(RadioButtonsRowSpacing));
  return outer;
}

}// namespace FredEmmott::GUI::Immediate