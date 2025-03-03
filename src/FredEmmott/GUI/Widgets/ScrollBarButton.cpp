// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBarButton.hpp"

#include <Windows.h>

#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
const auto ScrollBarButtonStyleClass = StyleClass::Make("ScrollBarButton");

std::chrono::steady_clock::duration GetRepeatDelay() {
  static std::optional<std::chrono::steady_clock::duration> sRet;
  if (sRet) {
    return *sRet;
  }

  int value {};
  SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &value, 0);

  // '3' is approximately 1 second (1hz)
  // '0' is approximately 250ms (4hz)

  constexpr auto Min = std::chrono::milliseconds(250);
  constexpr auto Ratio = (std::chrono::seconds(1) - Min) / 3;

  sRet = Min + (value * Ratio);

  return *sRet;
}

std::chrono::steady_clock::duration GetRepeatInterval() {
  static std::optional<std::chrono::steady_clock::duration> sRet;
  if (sRet) {
    return *sRet;
  }

  DWORD value {};
  SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &value, 0);

  // 31 is approximately 30hz
  // 0 is approximately 2.5hz

  constexpr auto MinHz = 2.5;
  constexpr auto MaxHz = 30;
  constexpr auto MinTime = std::chrono::milliseconds(1000) / MaxHz;
  constexpr auto MaxTime = std::chrono::milliseconds(1000) / MinHz;
  constexpr auto Ratio = (MaxTime - MinTime) / 31;

  sRet = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
    MaxTime - (value * Ratio));

  return *sRet;
}

}// namespace

ScrollBarButton::ScrollBarButton(
  std::function<void(const SkPoint&)> pressCallback,
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
void ScrollBarButton::BeforeFrame() {
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
    *mNextTick += GetRepeatInterval();
  }
}

Widget::EventHandlerResult ScrollBarButton::OnMouseButtonPress(
  const MouseEvent& e) {
  if (!e.IsValid()) {
    return EventHandlerResult::Default;
  }
  this->StartMouseCapture();
  if (mPressCallback) {
    mPressCallback(e.GetPosition());
  }
  if (mTickCallback) {
    mTickCallback();
  }
  mNextTick = std::chrono::steady_clock::now() + GetRepeatDelay();
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult ScrollBarButton::OnMouseButtonRelease(
  const MouseEvent& e) {
  if (!mNextTick) {
    return EventHandlerResult::Default;
  }

  mNextTick = std::nullopt;
  this->EndMouseCapture();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets