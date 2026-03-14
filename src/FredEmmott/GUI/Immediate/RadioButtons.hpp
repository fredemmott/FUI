// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/SelectionManager.hpp>

#include "ID.hpp"
#include "Result.hpp"
#include "selectable_key.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {

struct RadioButtonsWidgets {
  Widgets::Widget* mOuter {};
  Widgets::ISelectionContainer* mInner {};
};
RadioButtonsWidgets BeginRadioButtons(std::string_view, ID);

}// namespace immediate_detail

void EndRadioButtons();

/** Container for multiple RadioButton items.
 *
 * Not strictly required, however:
 * - without it, keyboard navigation (tabs/arrow keys) won't work
 * - it sets the flexbox gap property correctly
 * - optionally, it adds a header with the correct margin.
 */
template <selectable_key T>
Result<&EndRadioButtons> BeginRadioButtons(
  T* const selectedKey,
  const std::string_view title = {},
  const ID id = ID {std::source_location::current()}) {
  const auto [outer, inner] = immediate_detail::BeginRadioButtons(title, id);
  immediate_detail::SelectionManager<T>::BeginContainer(inner, selectedKey);
  return outer;
}
}// namespace FredEmmott::GUI::Immediate
