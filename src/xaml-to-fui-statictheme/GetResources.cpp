// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "GetResources.hpp"

#include <tinyxml.h>

#include <print>
#include <ranges>

#include "AliasMap.hpp"
#include "GetAlias.hpp"
#include "GetBoolean.hpp"
#include "GetColor.hpp"
#include "GetCornerRadius.hpp"
#include "GetLinearGradientBrush.hpp"
#include "GetNumber.hpp"
#include "GetSolidColorBrush.hpp"
#include "GetString.hpp"
#include "GetThickness.hpp"

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

void GetResource(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const std::string_view tagType {it.Value()};
  if (tagType == "StaticResource") {
    // We call this an 'alias'; handled separately
    return;
  }
  if (tagType == "Color") {
    GetColor(back, it);
    return;
  }
  if (tagType == "LinearGradientBrush") {
    GetLinearGradientBrush(back, it);
    return;
  }
  if (tagType == "SolidColorBrush") {
    GetSolidColorBrush(back, it);
    return;
  }
  if (tagType == "x:Double") {
    GetNumber(back, it, "double");
    return;
  }
  if (tagType == "x:Int32") {
    GetNumber(back, it, "int32_t");
    return;
  }
  if (tagType == "x:Boolean") {
    GetBoolean(back, it);
    return;
  }
  if (tagType == "x:String") {
    GetString(back, it);
    return;
  }
  if (tagType == "Thickness") {
    GetThickness(back, it);
    return;
  }
  if (tagType == "CornerRadius") {
    GetCornerRadius(back, it);
    return;
  }
  if (tagType == "FontWeight") {
    // TODO
    return;
  }
  if (tagType == "GridLength") {
    // TODO
    return;
  }
  if (tagType == "FontFamily" || tagType == "ListViewItemPresenterCheckMode") {
    // honestly probably not going to do.
    return;
  }

  std::println(
    stderr,
    "Unrecognized XAML theme tag type: {} {}",
    tagType,
    it.Attribute("x:Key"));
}

void GetThemeResources(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& theme) {
  for (auto child = theme.FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    GetResource(back, *child);
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
  for (auto child = root->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const auto& tag = child->ValueStr();
    if (!(tag.starts_with("x:") || tag == "Thickness"
          || tag == "CornerRadius")) {
      continue;
    }
    auto it = std::ranges::find(
      defaultResources, child->Attribute("x:Key"), &Resource::mName);
    if (it == defaultResources.end()) {
      GetResource(std::back_inserter(defaultResources), *child);
    }
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
    const auto defaultValue = ResolveValue("Dark", defaultIt);
    const auto lightValue = ResolveValue("Light", lightIt.value_or(defaultIt));
    const auto highContrastValue
      = ResolveValue("HighContrast", highContrastIt.value_or(defaultIt));

    if (defaultIt.IsLiteral()) {
      if (lightValue == defaultValue && highContrastValue == defaultValue) {
        back = defaultIt;
        continue;
      }
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
        defaultValue,
        lightValue,
        highContrastValue),
      .mType = std::format("Resource<{}>", defaultIt.mType),
    };
    ret.mDependencies = {defaultIt.mName};
    std::ranges::copy(
      defaultIt.mDependencies,
      std::inserter(ret.mDependencies, ret.mDependencies.begin()));
    if (lightIt) {
      ret.mDependencies.emplace(lightIt->mName);
      std::ranges::copy(
        lightIt->mDependencies,
        std::inserter(ret.mDependencies, ret.mDependencies.begin()));
    }
    if (highContrastIt) {
      ret.mDependencies.emplace(highContrastIt->mName);
      std::ranges::copy(
        highContrastIt->mDependencies,
        std::inserter(ret.mDependencies, ret.mDependencies.begin()));
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
