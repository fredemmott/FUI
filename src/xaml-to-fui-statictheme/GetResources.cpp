// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "GetResources.hpp"

#include <tinyxml.h>

#include <print>
#include <ranges>

#include "AliasMap.hpp"
#include "GetAcrylicBrush.hpp"
#include "GetAlias.hpp"
#include "GetBoolean.hpp"
#include "GetColor.hpp"
#include "GetCornerRadius.hpp"
#include "GetDesktopAcrylicBackdrop.hpp"
#include "GetLinearGradientBrush.hpp"
#include "GetNumber.hpp"
#include "GetSolidColorBrush.hpp"
#include "GetString.hpp"
#include "GetThickness.hpp"
#include "Theme.hpp"

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
  const Theme theme,
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  const std::string_view tagType {it.Value()};
  if (tagType == "StaticResource") {
    // We call this an 'alias'; these are handled separately
    return;
  }
  if (tagType == "DesktopAcrylicBackdrop") {
    GetDesktopAcrylicBackdrop(back, it);
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
  if (tagType == "AcrylicBrush") {
    GetAcrylicBrush(theme, back, it);
    return;
  }
  if (tagType == "x:Double") {
    GetNumber(back, it, "float");
    return;
  }
  if (tagType == "x:Int32") {
    GetNumber(back, it, "int32_t");
    return;
  }
  if (tagType == "GridLength") {
    // This could be 'double', 'starSizing', or 'auto' - for now,
    // everything we care about is a double
    GetNumber(back, it, "float");
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
  const Theme theme,
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& xml) {
  for (auto child = xml.FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    GetResource(theme, back, *child);
  }
}

std::string ResolveValue(std::string_view theme, const Resource& resource) {
  if (resource.mType != "Brush") {
    return resource.mValue;
  }
  return std::format(
    "ResolveThemedValue<StaticTheme::Theme::{}>({})", theme, resource.mValue);
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
  for (auto themeXml = themes->FirstChildElement("ResourceDictionary");
       themeXml;
       themeXml = themeXml->NextSiblingElement("ResourceDictionary")) {
    const std::string_view themeName = {themeXml->Attribute("x:Key")};

    std::vector<Resource>* resources {nullptr};
    Theme theme {};
    if (themeName == "Default") {
      resources = &defaultResources;
      theme = Theme::Dark;
    } else if (themeName == "Light") {
      resources = &lightResources;
      theme = Theme::Light;
    } else if (themeName == "HighContrast") {
      resources = &highContrastResources;
      theme = Theme::HighContrast;
    } else {
      throw std::runtime_error {
        std::format("Unrecognized theme name `{}`", themeName)};
    }

    GetThemeResources(theme, std::back_inserter(*resources), *themeXml);
  }
  // Unthemed resources within the top-level <ResourceDictionary>, but not
  // inside the <ResourceDictionary.ThemeDictionaries>
  for (auto child = root->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const auto& tag = child->ValueStr();
    if (tag == "StaticResource") {
      const auto target = child->Attribute("ResourceKey");
      back = {
        .mName = child->Attribute("x:Key"),
        .mValue = std::format("Theme::Get{0}()", target),
        .mType = std::format(
          "Resource<std::remove_cvref_t<decltype(Theme::Get{0}())>::value_"
          "type>",
          target),
        .mDependencies = {target},
        .mKind = Resource::Kind::Alias,
      };
      continue;
    }
    if (!(tag.starts_with("x:") || tag == "Thickness"
          || tag == "CornerRadius")) {
      continue;
    }
    auto it = std::ranges::find(
      defaultResources, child->Attribute("x:Key"), &Resource::mName);
    if (it == defaultResources.end()) {
      GetResource(
        Theme::Unthemed, std::back_inserter(defaultResources), *child);
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
