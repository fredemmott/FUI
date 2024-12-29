// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <tinyxml.h>

#include <string>
#include <unordered_map>

struct AliasMap {
  std::unordered_map<std::string, std::string> mDefault;
  std::unordered_map<std::string, std::string> mLight;
  std::unordered_map<std::string, std::string> mHighContrast;

  static AliasMap Load(const TiXmlElement& themes);
};
