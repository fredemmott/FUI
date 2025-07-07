// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ToggleSwitch.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {
struct OnOffTextResultMixin {
  /** Change the text displayed when the ToggleSwitch is on.
   *
   * Note Microsoft UI guidelines *strongly* recommend leaving this alone -
   * call `.Caption("descriptive text")` instead.
   */
  template <class Self>
  decltype(auto) OnText(this Self&& self, const std::string_view label) {
    Self::SetOnText(widget_from_result<Widgets::ToggleSwitch>(self), label);
    return std::forward<Self>(self);
  }

  /** Change the text displayed when the ToggleSwitch is on.
   *
   * Note Microsoft UI guidelines *strongly* recommend leaving this alone -
   * call `.Caption("descriptive text")` instead.
   */
  template <class Self, class... Args>
    requires(sizeof...(Args) >= 1)
  decltype(auto)
  OnText(this Self&& self, std::format_string<Args...> fmt, Args&&... args) {
    Self::SetOnText(
      widget_from_result<Widgets::ToggleSwitch>(self),
      std::format(fmt, std::forward<Args>(args)...));
    return std::forward<Self>(self);
  }

  /** Change the text displayed when the ToggleSwitch is off.
   *
   * Note Microsoft UI guidelines *strongly* recommend leaving this alone -
   * call `.Caption("descriptive text")` instead.
   */
  template <class Self>
  decltype(auto) OffText(this Self&& self, const std::string_view label) {
    Self::SetOffText(widget_from_result<Widgets::ToggleSwitch>(self), label);
    return std::forward<Self>(self);
  }

  /** Change the text displayed when the ToggleSwitch is off.
   *
   * Note Microsoft UI guidelines *strongly* recommend leaving this alone -
   * call `.Caption("descriptive text")` instead.
   */
  template <class Self, class... Args>
    requires(sizeof...(Args) >= 1)
  decltype(auto)
  OffText(this Self&& self, std::format_string<Args...> fmt, Args&&... args) {
    Self::SetOffText(
      widget_from_result<Widgets::ToggleSwitch>(self),
      std::format(fmt, std::forward<Args>(args)...));
    return std::forward<Self>(self);
  }

 private:
  static void SetOnText(Widgets::ToggleSwitch* self, std::string_view text);
  static void SetOffText(Widgets::ToggleSwitch* self, std::string_view text);
};
}// namespace immediate_detail

template <void (*TEndWidget)() = nullptr, class TValue = void, class... TMixins>
using ToggleSwitchResult = Result<
  TEndWidget,
  TValue,
  immediate_detail::CaptionResultMixin,
  TMixins...>;
template <void (*TEndWidget)() = nullptr, class TValue = void>
using LabeledToggleSwitchResult = ToggleSwitchResult<
  TEndWidget,
  TValue,
  immediate_detail::OnOffTextResultMixin>;

inline void EndToggleSwitch() {
  immediate_detail::EndWidget<Widgets::ToggleSwitch>();
}

ToggleSwitchResult<&EndToggleSwitch> BeginToggleSwitch(
  bool* isChanged,
  bool* isOn,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
LabeledToggleSwitchResult<nullptr, bool> ToggleSwitch(
  bool* isOn,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate