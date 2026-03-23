// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "RadioButtons.hpp"

#include <FredEmmott/GUI/StaticTheme/RadioButtons.hpp>
#include <FredEmmott/GUI/Widgets/Focusable.hpp>

#include "FredEmmott/GUI/Widgets/RadioButton.hpp"
#include "FredEmmott/GUI/detail/immediate/SelectionManager.hpp"
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
  explicit RadioButtonsInner(const std::size_t id)
    : Widget(id, InnerStyleClass, InnerStyle()),
      ISelectionContainer(this) {}

  std::vector<Widgets::ISelectionItem*> GetSelectionItems()
    const noexcept override {
    return GetLogicalChildren()
      | std::views::transform(&CastToSelectionItem<Widgets::RadioButton>)
      | std::ranges::to<std::vector>();
  }

 private:
};
}// namespace
namespace immediate_detail {
RadioButtonsWidgets BeginRadioButtons(
  const std::string_view title,
  const ID id) {
  using namespace StaticTheme::RadioButtons;
  using namespace PseudoClasses;

  const auto outer = BeginVStackPanel(id).Styled(
    Style().FlexDirection(YGFlexDirectionColumn).Gap(0));

  if (!title.empty()) {
    static const ImmutableStyle TitleStyle {
      Style()
        .Color(RadioButtonsHeaderForeground)
        .Margin(RadioButtonsTopHeaderMargin)
        .And(Disabled, Style().Color(RadioButtonsHeaderForegroundDisabled))};
    Label(title, ID {"RadioButtons/Title"}).Styled(TitleStyle);
  }

  const auto inner = immediate_detail::BeginWidget<RadioButtonsInner>(
    ID {"RadioButtons/Inner"});

  return {widget_from_result(outer), inner};
}
}// namespace immediate_detail

void EndRadioButtons() {
  immediate_detail::EndWidget<RadioButtonsInner>();
  EndVStackPanel();
}

}// namespace FredEmmott::GUI::Immediate