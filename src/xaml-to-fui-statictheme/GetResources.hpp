// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Resource.hpp"

#include <filesystem>
#include <iterator>
#include <vector>

void GetResources(
  std::back_insert_iterator<std::vector<Resource>>,
  const std::filesystem::path&);