// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBox.hpp"

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "Button.hpp"
#include "ComboBoxButton.hpp"
#include "ComboBoxItem.hpp"
#include "ComboBoxPopup.hpp"
#include "EnqueueAdditionalFrame.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {

const auto ComboBoxWidgetStyleClass = StyleClass::Make("ComboBoxWidget");

class ComboBoxWidget : public Widgets::Widget {
 public:
  using Widget::Widget;

  ComboBoxWidget() : Widget(0, {ComboBoxWidgetStyleClass}) {}
  bool mIsPopupOpen = false;
};
}// namespace

ComboBoxResult<bool> ComboBox(
  std::size_t* selectedIndex,
  std::span<const std::string_view> items,
  const ID id) {
  using namespace immediate_detail;
  FUI_ASSERT(selectedIndex, "A selected index is required");
  FUI_ASSERT(
    *selectedIndex < items.size(),
    "Selected index {} is >= items.size() {}",
    *selectedIndex,
    items.size());
  const auto widget = BeginWidget<ComboBoxWidget>(id);
  widget->SetBuiltInStyles(Style().Display(YGDisplayContents));

  const auto button = ComboBoxButton(items[*selectedIndex]);
  if (button.GetValue() /* clicked */) {
    widget->mIsPopupOpen = true;
  }

  bool changed = false;

  if (BeginComboBoxPopup(&widget->mIsPopupOpen)) {
    for (auto&& [i, item]: std::views::enumerate(items)) {
      if (ComboBoxItem((*selectedIndex) == i, item, ID {i})) {
        if (i != *selectedIndex) {
          changed = true;
          *selectedIndex = i;
        }
        widget->mIsPopupOpen = false;
      }
    }
    EndComboBoxPopup();
  }

  EndWidget<ComboBoxWidget>();
  return {widget_from_result(button), changed};
}

}// namespace FredEmmott::GUI::Immediate