// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <format>

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void PopID();
Result<&PopID, void, immediate_detail::WidgetlessResultMixin> PushID(
  const ID id);

template <class... Args>
auto PushID(std::format_string<Args...> fmt, Args&&... args) {
  return PushID(
    ID {std::hash<std::string_view> {}(
      std::format(fmt, std::forward<Args>(args)...))});
}

template <class T>
  requires requires(T v) {
    { std::hash<std::decay_t<T>> {}(v) } -> std::convertible_to<std::size_t>;
  }
auto PushID(T&& v) {
  return PushID(ID {std::hash<std::decay_t<T>> {}(std::forward<T>(v))});
}

}// namespace FredEmmott::GUI::Immediate