// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBox.hpp"

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "ComboBoxButton.hpp"
#include "ComboBoxItem.hpp"
#include "ComboBoxPopup.hpp"
#include "EnqueueAdditionalFrame.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {
class ComboBoxWidget : public Widgets::Widget {
 public:
  using Widget::Widget;

  bool mIsPopupOpen = false;
};
}// namespace

bool ComboBox(
  std::size_t* selectedIndex,
  std::span<std::string_view> items,
  ID id) {
  using namespace immediate_detail;
  FUI_ASSERT(selectedIndex, "A selected index is required");
  FUI_ASSERT(
    *selectedIndex < items.size(),
    "Selected index {} is >= items.size() {}",
    *selectedIndex,
    items.size());
  BeginWidget<ComboBoxWidget>(id);
  auto widget = GetCurrentParentNode<ComboBoxWidget>();
  widget->SetBuiltInStyles({{.mDisplay = YGDisplayContents}});

  if (ComboBoxButton("{}", items[*selectedIndex])) {
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
        EnqueueAdditionalFrame();
      }
    }
    EndComboBoxPopup();
  }

  EndWidget<ComboBoxWidget>();
  return changed;
}

}// namespace FredEmmott::GUI::Immediate