// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "CaptionResultMixin.hpp"

#include "FredEmmott/GUI/Immediate/Label.hpp"
#include "FredEmmott/GUI/assert.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

void CaptionResultMixin::AttachCaption(
  Widget* const target,
  const std::string_view label) {
  AttachCaption(
    target, label, ID {std::format("{}__GeneratedCaptionID", target->GetID())});
}

void CaptionResultMixin::AttachCaption(
  Widget* const target,
  const std::string_view label,
  const ID& id) {
  const auto caption = widget_from_result(
    Label(label, id).Caption().Styled(Style().MarginBottom(-4)));

  auto& siblings = tStack.back().mNewSiblings;
  FUI_ASSERT(siblings.back() == caption);
  // - search backwards as the widget will usually be the previous sibling, but
  //   will always be near the end
  // - skip 1 as the last is the caption widget
  const auto rit = std::ranges::find(
    std::ranges::next(siblings.rbegin()), siblings.rend(), target);
  FUI_ASSERT(rit != siblings.rend());
  // an iterator points at the gap before an element
  // a reverse iterator points at the gap after an element
  const auto it = std::ranges::prev(rit.base());
  std::ranges::rotate(it, siblings.end() - 1, siblings.end());
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail