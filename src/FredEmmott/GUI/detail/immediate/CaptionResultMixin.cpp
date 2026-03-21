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
  const auto it = std::ranges::find(siblings, target);
  FUI_ASSERT(it != siblings.end());
  std::ranges::rotate(it, siblings.end() - 1, siblings.end());
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail