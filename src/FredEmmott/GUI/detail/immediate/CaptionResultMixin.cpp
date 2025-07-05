// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "CaptionResultMixin.hpp"

#include "FredEmmott/GUI/assert.hpp"
namespace FredEmmott::GUI::Immediate::immediate_detail {

void CaptionResultMixin::AttachCaption(const std::string_view label) {
  const auto target = GetCurrentNode();
  AttachCaption(
    label, ID {std::format("{}__GeneratedCaptionID", target->GetID())});
}

void CaptionResultMixin::AttachCaption(
  const std::string_view label,
  const ID& id) {
  const auto target = GetCurrentNode();
  const auto caption = widget_from_result(
    Label(label, id).Caption().Styled({.mMarginBottom = -4}));
  auto& siblings = tStack.back().mNewSiblings;

  FUI_ASSERT(siblings.size() >= 2);
  auto& back = siblings.back();
  auto& prev = siblings.at(siblings.size() - 2);
  FUI_ASSERT(back == caption);
  FUI_ASSERT(prev == target);
  std::swap(back, prev);
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail