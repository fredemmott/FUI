// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetString.hpp"

#include <print>
#include <ranges>
#include <regex>

[[nodiscard]]
bool GetDuration(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  static const std::regex DurationPattern(
    "^(\\d{2}):(\\d{2}):(\\d{2})(\\.(\\d+))?$",
    std::regex_constants::ECMAScript);

  std::cmatch match;
  if (!std::regex_match(it.GetText(), match, DurationPattern)) {
    std::println(stderr, "Couldn't parse duration `{}`", it.GetText());
    return false;
  }

  const auto h = std::stoi(match[1]);
  const auto m = std::stoi(match[2]);
  const auto s = std::stoi(match[3]);

  const auto partialSeconds = match[5].str();

  std::vector<std::string> parts;
  if (h) {
    parts.push_back(std::format("std::chrono::hours{{ {} }}", h));
  }
  if (m) {
    parts.push_back(std::format("std::chrono::minutes{{ {} }}", m));
  }
  if (s) {
    parts.push_back(std::format("std::chrono::seconds {{ {} }}", s));
  }

  if (!partialSeconds.empty()) {
    const auto digits = partialSeconds.length();
    if (digits <= 3) {
      const auto value = std::stoul(std::format("{:0<3}", partialSeconds));
      parts.push_back(std::format("std::chrono::milliseconds {{ {} }}", value));
    } else if (digits <= 6) {
      const auto value = std::stoul(std::format("{:0<6}", partialSeconds));
      parts.push_back(std::format("std::chrono::microseconds {{ {} }}", value));
    } else if (digits <= 9) {
      const auto value = std::stoul(std::format("{:0<9}", partialSeconds));
      parts.push_back(std::format("std::chrono::nanoseconds {{ {} }}", value));
    } else {
      throw std::runtime_error(
        std::format(
          "Can't parse duration value `{}` - unhandled precision",
          it.GetText()));
    }
  }

  const auto value
    = std::ranges::to<std::string>(std::views::join_with(parts, '+'));

  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::format("/* '{}' */ {}", it.GetText(), value),
    .mType = "std::chrono::steady_clock::duration",
    .mKind = Resource::Kind::Literal,
  };

  return true;
}

void GetString(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const auto key = std::string_view {it.Attribute("x:Key")};
  if (key.ends_with("Duration") || key.ends_with("Time")) {
    if (GetDuration(back, it)) {
      return;
    }
  }

  if (key.ends_with("KeySpline")) {
    back = {
      .mName = it.Attribute("x:Key"),
      .mValue = std::format("{{ {} }}", it.GetText()),
      .mType = "std::array<float, 4>",
      .mKind = Resource::Kind::Literal,
    };
    return;
  }

  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::format("\"{}\"", it.GetText()),
    .mType = "std::string",
  };
}
