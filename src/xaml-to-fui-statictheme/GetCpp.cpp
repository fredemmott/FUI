// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetCpp.hpp"

#include <fmt/format.h>

#include <ranges>
#include <vector>

std::string GetCpp(const Metadata& meta, const std::span<Resource> resources) {
  std::vector<std::string> resourceGetters;
  resourceGetters.reserve(resources.size());
  for (auto&& resource: resources) {
    if (resource.IsLiteral()) {
      continue;
    }
    const std::string type = resource.IsAlias()
      ? fmt::format("Theme::{}_t", resource.mName)
      : resource.mType;

    resourceGetters.push_back(
      fmt::format(
        R"EOF(
const {TYPE}* Theme::Get{NAME}() {{
  static const {TYPE} sValue = {VALUE};
  return &sValue;
}}

)EOF",
        fmt::arg("TYPE", type),
        fmt::arg("NAME", resource.mName),
        fmt::arg("VALUE", resource.mValue),
        nullptr));
  }

  return fmt::format(
    R"EOF(
{PARENT_INCLUDE}
#include "{COMPONENT}.hpp"

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/SystemTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp>

#include <thread>

namespace {NAMESPACE}::{DETAIL_NAMESPACE} {{

{PARENT_USING}

using enum SystemTheme::ColorType;

{RESOURCE_GETTERS}

}} // namespace {NAMESPACE}::{DETAIL_NAMESPACE}
)EOF",
    fmt::arg(
      "PARENT_INCLUDE",
      meta.mParent.empty()
        ? "// #include PARENT"
        : fmt::format(
            "#include <FredEmmott/GUI/StaticTheme/{}.hpp>", meta.mParent)),
    fmt::arg("COMPONENT", meta.mComponent),
    fmt::arg("NAMESPACE", meta.mNamespace),
    fmt::arg("DETAIL_NAMESPACE", meta.mDetailNamespace),
    fmt::arg(
      "PARENT_USING",
      meta.mParent.empty() ? "// using namespace PARENT;"
                           : fmt::format("using namespace {};", meta.mParent)),
    fmt::arg(
      "RESOURCE_GETTERS",
      std::ranges::to<std::string>(
        std::views::join_with(resourceGetters, '\n'))),
    nullptr);
}
