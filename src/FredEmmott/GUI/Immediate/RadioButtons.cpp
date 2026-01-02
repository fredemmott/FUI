// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "RadioButtons.hpp"

#include <FredEmmott/GUI/StaticTheme/RadioButtons.hpp>
#include <FredEmmott/GUI/Widgets/Focusable.hpp>

#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {

auto& InnerStyle() {
  static const ImmutableStyle ret {
    Style()
      .FlexDirection(YGFlexDirectionColumn)
      .Gap(StaticTheme::RadioButtons::RadioButtonsRowSpacing),
  };
  return ret;
}

constexpr LiteralStyleClass InnerStyleClass {"RadioButtons/Inner"};

class RadioButtonsInner : public Widgets::Widget,
                          public Widgets::ISelectionContainer {
 public:
  explicit RadioButtonsInner(std::size_t id)
    : Widget(id, InnerStyleClass, InnerStyle()) {}
};
}// namespace

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

  immediate_detail::BeginWidget<RadioButtonsInner>(ID {"RadioButtons/Inner"});
  return outer;
}

void EndRadioButtons() {
  immediate_detail::EndWidget<RadioButtonsInner>();
  EndVStackPanel();
}

}// namespace FredEmmott::GUI::Immediate