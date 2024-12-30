// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <span>
#include <string>

#include "Metadata.hpp"
#include "Resource.hpp"

struct HppData {
  Metadata mMetadata;

  std::string mParentInclude;
  std::string mParent;
  std::vector<std::string> mMembers;
  std::vector<std::string> mConstants;
};

HppData GetHppData(const Metadata&, const std::span<Resource>&);

std::string GetHpp(const HppData&);
std::string GetDetailHpp(const HppData&);
