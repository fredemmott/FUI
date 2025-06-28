// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBarButton.hpp"

#include <Windows.h>

#include "FredEmmott/GUI/SystemSettings.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
const auto ScrollBarButtonStyleClass = StyleClass::Make("ScrollBarButton");

}// namespace

ScrollBarButton::ScrollBarButton(
  std::function<void(const Point&)> pressCallback,
  std::function<void()> tickCallback,
  std::size_t id)
  : Widget(id, {ScrollBarButtonStyleClass}),
    mPressCallback(pressCallback),
    mTickCallback(tickCallback) {}

ScrollBarButton::~ScrollBarButton() = default;

void ScrollBarButton::SetText(std::string_view text) {
  auto label = new Label(0);
  label->SetText(text);
  this->SetChildren({label});
}

FrameRateRequirement ScrollBarButton::GetFrameRateRequirement() const noexcept {
  if (mNextTick) {
    return FrameRateRequirement::SmoothAnimation;
  }
  return Widget::GetFrameRateRequirement();
}
void ScrollBarButton::Tick() {
  if (!mNextTick) {
    return;
  }
  const auto tick = mTickCallback;
  if (!tick) {
    return;
  }

  const auto now = std::chrono::steady_clock::now();
  while (now > mNextTick.value()) {
    tick();
    *mNextTick += SystemSettings::Get().GetKeyboardRepeatInterval();
  }
}

Widget::EventHandlerResult ScrollBarButton::OnMouseButtonPress(
  const MouseEvent& e) {
  if (this->IsDisabled() || !e.IsValid()) {
    return EventHandlerResult::Default;
  }
  (void)Widget::OnMouseButtonPress(e);

  this->StartMouseCapture();
  if (mPressCallback) {
    mPressCallback(e.GetPosition());
  }
  if (mTickCallback) {
    mTickCallback();
  }
  mNextTick = std::chrono::steady_clock::now()
    + SystemSettings::Get().GetKeyboardRepeatDelay();
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult ScrollBarButton::OnMouseButtonRelease(
  const MouseEvent& e) {
  if (this->IsDisabled() || !mNextTick) {
    return EventHandlerResult::Default;
  }
  (void)Widget::OnMouseButtonRelease(e);

  mNextTick = std::nullopt;
  this->EndMouseCapture();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets