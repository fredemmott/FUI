// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "HyperlinkButton.hpp"

#include "Button.hpp"
#include "FredEmmott/GUI/Widgets/HyperlinkButton.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {
HyperlinkButtonResult<void>
HyperlinkButton(bool* clicked, const std::string_view label, const ID id) {
  using namespace immediate_detail;
  auto ret = BeginWidget<Widgets::HyperlinkButton>(id);
  Label(label).Styled(Style().TextAlign(TextAlign::Center).FlexGrow(1));
  if (clicked) {
    *clicked = ret->ConsumeWasActivated();
  }
  EndWidget<Widgets::HyperlinkButton>();
  return {ret};
}

HyperlinkButtonResult<bool> HyperlinkButton(
  const std::string_view label,
  const ID id) {
  bool clicked = false;
  auto ret = HyperlinkButton(&clicked, label, id);
  return {ret, clicked};
}
}// namespace FredEmmott::GUI::Immediate
