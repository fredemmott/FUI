// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetAlias.hpp"

#include <print>

std::string GetAliasTargetType(std::string_view value) {
  if (value.starts_with("System") && value.ends_with("Color")) {
    return "Color";
  }
  return std::format("std::decay_t<decltype(*{})>::value_type", value);
}

std::string GetAliasValue(
  const std::string_view theme,
  const std::string_view value) {
  if (
    value.starts_with("System") && value.ends_with("Color")
    && value != "SystemControlTransparentColor"
    && value != "SystemControlDisabledTransparentColor") {
    return std::string {value};
  }
  return std::format("{}->Resolve(StaticTheme::Theme::{})", value, theme);
}

void GetAlias(
  std::back_insert_iterator<std::vector<Resource>> back,
  const AliasMap& aliasMap,
  const std::string& key) {
  const auto defaultIt = aliasMap.mDefault.at(key);
  const auto lightIt
    = aliasMap.mLight.contains(key) ? aliasMap.mLight.at(key) : defaultIt;
  const auto highContrastIt = aliasMap.mHighContrast.contains(key)
    ? aliasMap.mHighContrast.at(key)
    : defaultIt;

  const auto defaultValue = GetAliasValue("Dark", defaultIt);
  const auto lightValue = GetAliasValue("Light", lightIt);
  const auto highContrastValue = GetAliasValue("HighContrast", highContrastIt);

  Resource ret {
    .mName = key,
    .mValue = std::format(
      R"EOF(
{{
  .mDefault = {},
  .mLight = {},
  .mHighContrast = {},
}}
)EOF",
      defaultValue,
      lightValue,
      highContrastValue),
    .mType = std::format(
      R"EOF(
ResourceSupertype<
  {},
  {},
  {}
>::type
)EOF",
      GetAliasTargetType(defaultIt),
      GetAliasTargetType(lightIt),
      GetAliasTargetType(highContrastIt)),
    .mIsAlias = true,
    .mDependencies = {defaultIt, lightIt, highContrastIt},
  };
  back = std::move(ret);
}
