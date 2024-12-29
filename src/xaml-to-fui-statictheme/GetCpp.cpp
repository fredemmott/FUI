// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetCpp.hpp"

#include <fmt/format.h>

#include <ranges>
#include <vector>

std::string GetCpp(const Metadata& meta, const std::span<Resource> resources) {
  std::vector<std::string> resourceGetters;
  for (auto&& resource: resources) {
    const std::string type = resource.mIsAlias
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
#include "{COMPONENT}.hpp"

#include <skia/core/SkColor.h>
#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/SystemTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp>

#include <thread>

namespace {NAMESPACE}::{DETAIL_NAMESPACE} {{

using enum SystemTheme::ColorType;

const Theme* Theme::GetInstance() {{
  static std::once_flag sOnce;
  static std::unique_ptr<Theme> sInstance;
  std::call_once(sOnce, [&it = sInstance]() {{
    it = std::make_unique<Theme>();
  }});
  return sInstance.get();
}}

{RESOURCE_GETTERS}

}} // namespace {NAMESPACE}::{DETAIL_NAMESPACE}
)EOF",
    fmt::arg("COMPONENT", meta.mComponent),
    fmt::arg("NAMESPACE", meta.mNamespace),
    fmt::arg("DETAIL_NAMESPACE", meta.mDetailNamespace),
    fmt::arg(
      "RESOURCE_GETTERS",
      std::ranges::to<std::string>(
        std::views::join_with(resourceGetters, '\n'))),
    nullptr);
}
