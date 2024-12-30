// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetHpp.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
#include <vector>

HppData GetHppData(const Metadata& meta, const std::span<Resource>& resources) {
  const auto parentInclude = meta.mParent.empty()
    ? "// #include PARENT"
    : fmt::format("#include <FredEmmott/GUI/StaticTheme/{}.hpp>", meta.mParent);
  const auto parent = meta.mParent.empty()
    ? "/* : PARENT */"
    : fmt::format(": {0}::detail_StaticTheme_{0}::Theme", meta.mParent);

  struct Constant {
    std::string mName;
    std::string mCode;
  };
  std::vector<Constant> constants;
  std::vector<std::string> members;

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
      constants.emplace_back(
        resource.mName,
        fmt::format(
          "constexpr {} {} {{ {} }};",
          resource.mType,
          resource.mName,
          resource.mValue));
    } else {
      constants.emplace_back(
        resource.mName,
        fmt::format(
          "inline const auto {0} = {1}::Theme::GetInstance()->{0};",
          resource.mName,
          meta.mDetailNamespace));
    }
  }
  std::ranges::sort(constants, {}, &Constant::mName);
  return {
    .mMetadata = meta,
    .mParentInclude = parentInclude,
    .mParent = parent,
    .mMembers = members,
    .mConstants = constants | std::views::transform(&Constant::mCode)
      | std::ranges::to<std::vector<std::string>>(),
  };
}

std::string GetHpp(const HppData& data) {
  return fmt::format(
    R"EOF(
#pragma once

#include "detail/{COMPONENT}.hpp"

namespace {NAMESPACE} {{

{CONSTANTS}

}} // namespace {NAMESPACE}
)EOF",
    fmt::arg("COMPONENT", data.mMetadata.mComponent),
    fmt::arg("NAMESPACE", data.mMetadata.mNamespace),
    fmt::arg(
      "CONSTANTS",
      std::ranges::to<std::string>(
        std::views::join_with(data.mConstants, '\n'))),
    nullptr);
}

std::string GetDetailHpp(const HppData& data) {
  return fmt::format(
    R"EOF(
#pragma once

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/StaticTheme/Resource.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/ResourceSupertype.hpp>

{PARENT_INCLUDE}

#include <array>
#include <chrono>

namespace {NAMESPACE}::{DETAIL_NAMESPACE} {{

struct Theme {PARENT} {{
  public:
    static const Theme* GetInstance();

    {MEMBERS}
}}; // struct Theme

}} // namespace {NAMESPACE}::{DETAIL_NAMESPACE}
)EOF",
    fmt::arg("PARENT_INCLUDE", data.mParentInclude),
    fmt::arg("PARENT", data.mParent),
    fmt::arg("NAMESPACE", data.mMetadata.mNamespace),
    fmt::arg("DETAIL_NAMESPACE", data.mMetadata.mDetailNamespace),
    fmt::arg(
      "MEMBERS",
      std::ranges::to<std::string>(std::views::join_with(data.mMembers, '\n'))),
    nullptr);
}
