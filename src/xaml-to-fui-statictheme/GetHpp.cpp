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
  std::vector<std::string> wrapperTypes;
  constants.reserve(resources.size());
  members.reserve(resources.size());
  wrapperTypes.reserve(resources.size());

  for (auto&& resource: resources) {
    std::string type = resource.mType;
    if (resource.IsAlias()) {
      type = fmt::format("{}_t", resource.mName);
      members.push_back(fmt::format("using {} = {};", type, resource.mType));
    }

    if (resource.IsLiteral()) {
      constants.emplace_back(
        resource.mName,
        fmt::format(
          "constexpr {} {} {{ {} }};",
          resource.mType,
          resource.mName,
          resource.mValue));
      continue;
    }
    members.push_back(
      fmt::format("static const {}* Get{}();", type, resource.mName));

    constants.emplace_back(
      resource.mName,
      fmt::format(
        "constexpr {DETAIL_NS}::{NAME}_t {NAME};",
        fmt::arg("NAME", resource.mName),
        fmt::arg("DETAIL_NS", meta.mDetailNamespace)));
    wrapperTypes.emplace_back(
      fmt::format(
        R"EOF(
struct {NAME}_t {{
  using type = decltype(Theme::Get{NAME}());
  using value_type = std::remove_pointer_t<type>::value_type;

  operator type() const {{ return Theme::Get{NAME}(); }}
  operator const value_type&() const {{ return *Theme::Get{NAME}()->Resolve(); }}
  type operator->() const {{
    return Theme::Get{NAME}();
  }}
}};
)EOF",
        fmt::arg("NAME", resource.mName),
        fmt::arg("TYPE", resource.IsAlias() ? resource.mType : type)));
  }
  std::ranges::sort(constants, {}, &Constant::mName);
  return {
    .mMetadata = meta,
    .mParentInclude = parentInclude,
    .mParent = parent,
    .mMembers = members,
    .mConstants = constants | std::views::transform(&Constant::mCode)
      | std::ranges::to<std::vector<std::string>>(),
    .mWrapperTypes = wrapperTypes,
  };
}

std::string GetHpp(const HppData& data) {
  const auto handWrittenHeader = fmt::format(
    "<FredEmmott/GUI/StaticTheme/detail/{}.handwritten.hpp>",
    data.mMetadata.mComponent);
  return fmt::format(
    R"EOF(
#pragma once

#include "detail/{COMPONENT}.hpp"

namespace {NAMESPACE} {{

{CONSTANTS}

}} // namespace {NAMESPACE}

#if __has_include({MANUAL_HEADER})
#include {MANUAL_HEADER}
#endif
)EOF",
    fmt::arg("COMPONENT", data.mMetadata.mComponent),
    fmt::arg("NAMESPACE", data.mMetadata.mNamespace),
    fmt::arg("MANUAL_HEADER", handWrittenHeader),
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
    Theme() = delete;

    {MEMBERS}
}}; // struct Theme

{WRAPPER_TYPES}

}} // namespace {NAMESPACE}::{DETAIL_NAMESPACE}
)EOF",
    fmt::arg("PARENT_INCLUDE", data.mParentInclude),
    fmt::arg("PARENT", data.mParent),
    fmt::arg("NAMESPACE", data.mMetadata.mNamespace),
    fmt::arg("DETAIL_NAMESPACE", data.mMetadata.mDetailNamespace),
    fmt::arg(
      "MEMBERS",
      std::ranges::to<std::string>(std::views::join_with(data.mMembers, '\n'))),
    fmt::arg(
      "WRAPPER_TYPES",
      std::ranges::to<std::string>(
        std::views::join_with(data.mWrapperTypes, '\n'))),
    nullptr);
}
