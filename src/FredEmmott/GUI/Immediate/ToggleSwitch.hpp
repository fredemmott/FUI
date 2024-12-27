// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ToggleSwitch.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

struct FormattedString {
  FormattedString() = delete;
  FormattedString(std::string_view text) : mText(std::string {text}) {};

  template <class... Args>
  FormattedString(std::format_string<Args...> fmt, Args&&... args)
    : mText(std::format(fmt, std::forward<Args>(args)...)) {
  }

  bool operator==(const FormattedString&) const noexcept = default;

  operator std::string_view() const noexcept {
    return mText;
  }

 private:
  std::string mText;
};

void BeginToggleSwitch(
  bool* isChanged,
  bool* isOn,
  const Widgets::WidgetStyles& styles = {});

inline void EndToggleSwitch() {
  immediate_detail::EndWidget<Widgets::ToggleSwitch>();
}

[[nodiscard]]
bool ToggleSwitch(
  bool* isOn,
  const Widgets::WidgetStyles& styles = {},
  const FormattedString& offText = {"Off"},
  const FormattedString& onText = {"On"});

}// namespace FredEmmott::GUI::Immediate