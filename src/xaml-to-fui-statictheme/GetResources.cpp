// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "GetResources.hpp"

#include <tinyxml.h>

#include <ranges>

#include "AliasMap.hpp"
#include "GetAlias.hpp"
#include "GetColor.hpp"
#include "GetLinearGradientBrush.hpp"
#include "GetSolidColorBrush.hpp"

void GetResources(
  std::back_insert_iterator<std::vector<Resource>>,
  const TiXmlDocument& doc);

void GetResources(
  std::back_insert_iterator<std::vector<Resource>> back,
  const std::filesystem::path& path) {
  TiXmlDocument doc(path.string());
  if (!doc.LoadFile()) {
    throw std::runtime_error {std::format(
      "Failed to load XML file `{}`: {}", path.string(), doc.ErrorDesc())};
  }
  GetResources(back, doc);
}

void GetThemeResources(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& theme) {
  for (auto child = theme.FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const std::string_view tagType {child->Value()};
    if (tagType == "Color") {
      GetColor(back, *child);
      continue;
    }
    if (tagType == "LinearGradientBrush") {
      GetLinearGradientBrush(back, *child);
    }
    if (tagType == "SolidColorBrush") {
      GetSolidColorBrush(back, *child);
      continue;
    }
  }
}

std::string ResolveValue(std::string_view theme, const Resource& resource) {
  if (resource.mType != "Brush") {
    return resource.mValue;
  }
  return std::format(
    "ResolveBrush<StaticTheme::Theme::{}>({})", theme, resource.mValue);
}

void GetResources(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlDocument& doc) {
  const auto root = doc.FirstChildElement("ResourceDictionary");
  const auto themes
    = root->FirstChildElement("ResourceDictionary.ThemeDictionaries");

  std::vector<Resource> defaultResources;
  std::vector<Resource> lightResources;
  std::vector<Resource> highContrastResources;
  for (auto theme = themes->FirstChildElement("ResourceDictionary"); theme;
       theme = theme->NextSiblingElement("ResourceDictionary")) {
    const std::string_view themeName = {theme->Attribute("x:Key")};

    std::vector<Resource>* resources {nullptr};
    if (themeName == "Default") {
      resources = &defaultResources;
    } else if (themeName == "Light") {
      resources = &lightResources;
    } else if (themeName == "HighContrast") {
      resources = &highContrastResources;
    } else {
      throw std::runtime_error {
        std::format("Unrecognized theme name `{}`", themeName)};
    }

    GetThemeResources(std::back_inserter(*resources), *theme);
  }

  for (auto&& defaultIt: defaultResources) {
    std::optional<Resource> lightIt;
    std::optional<Resource> highContrastIt;

    if (const auto it
        = std::ranges::find(lightResources, defaultIt.mName, &Resource::mName);
        it != lightResources.end()) {
      lightIt = *it;
    }
    if (const auto it = std::ranges::find(
          highContrastResources, defaultIt.mName, &Resource::mName);
        it != highContrastResources.end()) {
      highContrastIt = *it;
    }

    Resource ret {
      .mName = defaultIt.mName,
      .mValue = std::format(
        R"EOF(
{{
  .mDefault = {},
  .mLight = {},
  .mHighContrast = {},
}}
)EOF",
        ResolveValue("Dark", defaultIt),
        ResolveValue("Light", lightIt.value_or(defaultIt)),
        ResolveValue("HighContrast", highContrastIt.value_or(defaultIt))),
      .mType = std::format("Resource<{}>", defaultIt.mType),
    };
    ret.mDependencies = {defaultIt.mName};
    std::ranges::copy(
      defaultIt.mDependencies, std::back_inserter(ret.mDependencies));
    if (lightIt) {
      ret.mDependencies.push_back(lightIt->mName);
      std::ranges::copy(
        lightIt->mDependencies, std::back_inserter(ret.mDependencies));
    }
    if (highContrastIt) {
      ret.mDependencies.push_back(highContrastIt->mName);
      std::ranges::copy(
        highContrastIt->mDependencies, std::back_inserter(ret.mDependencies));
    }
    back = std::move(ret);
  }

  const auto aliasMap = AliasMap::Load(*themes);
  auto aliasKeys
    = std::views::keys(aliasMap.mDefault) | std::ranges::to<std::vector>();
  std::ranges::sort(aliasKeys, std::less {});
  for (auto&& key: aliasKeys) {
    GetAlias(back, aliasMap, key);
  }
}
