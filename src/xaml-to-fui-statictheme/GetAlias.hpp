// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <tinyxml.h>

#include <iterator>
#include <vector>

#include "AliasMap.hpp"
#include "Resource.hpp"

/// <StaticResource> in the XML, but that naming seems ambiguous in this context
void GetAlias(
  std::back_insert_iterator<std::vector<Resource>> back,
  const AliasMap& aliasMap,
  const std::string& key);
