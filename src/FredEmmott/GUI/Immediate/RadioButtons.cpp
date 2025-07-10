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

  auto outer = BeginVStackPanel(id).Styled({
    .mFlexDirection = YGFlexDirectionColumn,
    .mGap = 0,
  });

  if (!title.empty()) {
    Label(title, ID {"RadioButtons/Title"}).Styled({
      .mColor = RadioButtonsHeaderForeground,
      .mMarginBottom = RadioButtonsTopHeaderMarginBottom,
      .mMarginLeft = RadioButtonsTopHeaderMarginLeft,
      .mMarginRight = RadioButtonsTopHeaderMarginRight,
      .mMarginTop = RadioButtonsTopHeaderMarginTop,
      .mAnd = {
        { Disabled, {
          .mColor = RadioButtonsHeaderForegroundDisabled,
        }},
      },
    });
  }

  BeginVStackPanel(ID {"RadioButtons/Content"})
    .Styled({
      .mGap = RadioButtonsRowSpacing,
    });
  return outer;
}

}// namespace FredEmmott::GUI::Immediate