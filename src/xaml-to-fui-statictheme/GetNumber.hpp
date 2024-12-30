// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <iterator>
#include <vector>
#include <tinyxml.h>

#include "Resource.hpp"

void GetNumber(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it);
