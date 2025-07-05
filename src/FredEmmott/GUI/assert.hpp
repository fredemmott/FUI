// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <format>
#include <source_location>

namespace FredEmmott::GUI {
template <class... Args>
[[noreturn]]
void assert_fail(
  const std::source_location& location,
  std::string_view condition,
  std::format_string<Args...> fmt,
  Args&&... args) {
  const auto prefix = std::format(
    "Assertion failed: `{}` at {}:{}\n",
    condition,
    location.file_name(),
    location.line());
  if (fmt.get().empty()) {
    throw std::logic_error(prefix);
  }
  throw std::logic_error(
    prefix + std::format(fmt, std::forward<Args>(args)...));
}

[[noreturn]]
inline void assert_fail(
  const std::source_location& location,
  std::string_view condition) {
  assert_fail(location, condition, "");
}
}// namespace FredEmmott::GUI

#ifdef _MSVC_TRADITIONAL
#define FUI_ALWAYS_ASSERT(condition, ...) \
  if (!(condition)) [[unlikely]] { \
    assert_fail(std::source_location::current(), #condition, ##__VA_ARGS__); \
  }
#else
#define FUI_ALWAYS_ASSERT(condition, ...) \
  if (!(condition)) [[unlikely]] { \
    assert_fail( \
      std::source_location::current(), #condition __VA_OPT__(, ) __VA_ARGS__); \
  }

#endif

#define FUI_ASSERT(...) \
  if constexpr (FredEmmott::GUI::Config::Debug) { \
    FUI_ALWAYS_ASSERT(__VA_ARGS__); \
  }
