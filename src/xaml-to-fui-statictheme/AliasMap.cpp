// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "AliasMap.hpp"

AliasMap AliasMap::Load(const TiXmlElement& themes) {
  AliasMap ret {};
  for (auto theme = themes.FirstChildElement("ResourceDictionary"); theme;
       theme = theme->NextSiblingElement("ResourceDictionary")) {
    const std::string_view themeName {theme->Attribute("x:Key")};

    std::unordered_map<std::string, std::string>* aliases {nullptr};
    if (themeName == "Default") {
      aliases = &ret.mDefault;
    } else if (themeName == "Light") {
      aliases = &ret.mLight;
    } else if (themeName == "HighContrast") {
      aliases = &ret.mHighContrast;
    } else {
      throw std::runtime_error {
        std::format("Unrecognized theme name `{}`", themeName)};
    }

    for (auto it = theme->FirstChildElement("StaticResource"); it;
         it = it->NextSiblingElement("StaticResource")) {
      aliases->emplace(it->Attribute("x:Key"), it->Attribute("ResourceKey"));
    }
  }
  return ret;
}
