// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetHpp.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
#include <vector>

std::string GetHpp(const Metadata& meta, const std::span<Resource> resources) {
  const auto parentInclude = meta.mParent.empty()
    ? "// #include PARENT"
    : fmt::format("#include <FredEmmott/GUI/StaticTheme/{}.hpp>", meta.mParent);
  const auto parent = meta.mParent.empty()
    ? "/* : PARENT */"
    : fmt::format(": {0}::detail_StaticTheme_{0}::Theme", meta.mParent);

  std::vector<std::string> members;
  std::vector<std::string> constants;

  for (auto&& resource: resources) {
    std::string type = resource.mType;
    if (resource.IsAlias()) {
      type = fmt::format("{}_t", resource.mName);
      members.push_back(fmt::format("using {} = {};", type, resource.mType));
    }

    members.push_back(fmt::format("const {}* Get{}();", type, resource.mName));
    members.push_back(
      fmt::format("const {0}* {1} = {{ Get{1}() }};", type, resource.mName));

    if (resource.IsLiteral()) {
      constants.push_back(
        fmt::format(
          "constexpr {} {} {{ {} }};",
          resource.mType,
          resource.mName,
          resource.mValue));
    } else {
      constants.push_back(
        fmt::format(
          "inline const auto {0} = {1}::Theme::GetInstance()->{0};",
          resource.mName,
          meta.mDetailNamespace));
    }
  }
  std::ranges::sort(constants);

  return fmt::format(
    R"EOF(
#pragma once

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/StaticTheme/Resource.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/ResourceSupertype.hpp>
{PARENT_INCLUDE}

namespace {NAMESPACE}::{DETAIL_NAMESPACE} {{

struct Theme {PARENT} {{
  public:
    static const Theme* GetInstance();

    {MEMBERS}
}}; // struct Theme

}} // namespace {NAMESPACE}::{DETAIL_NAMESPACE}

namespace {NAMESPACE} {{

{CONSTANTS}

}} // namespace {NAMESPACE}
)EOF",
    fmt::arg("PARENT_INCLUDE", parentInclude),
    fmt::arg("PARENT", parent),
    fmt::arg("NAMESPACE", meta.mNamespace),
    fmt::arg("DETAIL_NAMESPACE", meta.mDetailNamespace),
    fmt::arg(
      "MEMBERS",
      std::ranges::to<std::string>(std::views::join_with(members, '\n'))),
    fmt::arg(
      "CONSTANTS",
      std::ranges::to<std::string>(std::views::join_with(constants, '\n'))),
    nullptr);
}
