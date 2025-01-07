// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <tinyxml.h>

#include <iterator>
#include <vector>

#include "Resource.hpp"

void GetAcrylicBrush(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it);