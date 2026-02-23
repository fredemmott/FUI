// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitch.hpp"

#include <FredEmmott/GUI/Immediate/Label.hpp>
#include <FredEmmott/GUI/Widgets/Label.hpp>

namespace FredEmmott::GUI::Immediate {

namespace {
struct ToggleSwitchContext : Widgets::Context {
  Widgets::Label* mLabel {nullptr};
  std::string mOnText {"On"};
  std::string mOffText {"Off"};
};
}// namespace

void immediate_detail::OnOffTextResultMixin::SetOnText(
  Widgets::ToggleSwitch* self,
  const std::string_view text) {
  const auto ctx = self->GetContext<ToggleSwitchContext>();
  ctx->mOnText = text;
  if (self->IsOn()) {
    ctx->mLabel->SetText(text);
  }
}

void immediate_detail::OnOffTextResultMixin::SetOffText(
  Widgets::ToggleSwitch* self,
  const std::string_view text) {
  const auto ctx = self->GetContext<ToggleSwitchContext>();
  ctx->mOffText = text;
  if (!self->IsOn()) {
    ctx->mLabel->SetText(text);
  }
}

ToggleSwitchResult<&EndToggleSwitch>
BeginToggleSwitch(bool* pIsChanged, bool* pIsOn, const ID id) {
  using namespace immediate_detail;
  using Widgets::ToggleSwitch;

  const auto toggle = BeginWidget<ToggleSwitch>(id);
  const auto isChanged = toggle->ConsumeWasChanged();
  if (pIsChanged) {
    *pIsChanged = isChanged;
  }
  if (pIsOn && isChanged) {
    *pIsOn = toggle->IsOn();
  } else if (pIsOn) {
    toggle->SetIsOn(*pIsOn);
    std::ignore = toggle->ConsumeWasChanged();
  }
  return {toggle};
}

LabeledToggleSwitchResult<nullptr, bool> ToggleSwitch(
  bool* pIsOn,
  const ID id) {
  bool isOn {false};
  if (pIsOn) [[likely]] {
    isOn = *pIsOn;
  }
  bool isChanged {false};
  const auto toggle = BeginToggleSwitch(&isChanged, &isOn, id);
  const auto toggleWidget = widget_from_result<Widgets::ToggleSwitch>(toggle);

  if (pIsOn) [[likely]] {
    *pIsOn = isOn;
  }

  std::string_view onText = "On";
  std::string_view offText = "Off";
  if (const auto ctx = toggleWidget->GetContext<ToggleSwitchContext>()) {
    onText = ctx->mOnText;
    offText = ctx->mOffText;
  }
  auto label
    = Label(isOn ? onText : offText, ID {"__GeneratedToggleSwitchLabel"});
  widget_from_result(toggle)->SetContextIfUnset([&label] {
    auto ret = std::make_unique<ToggleSwitchContext>();
    ret->mLabel = widget_from_result<Widgets::Label>(label);
    return ret;
  });

  EndToggleSwitch();
  return {toggle, isChanged};
}

}// namespace FredEmmott::GUI::Immediate
