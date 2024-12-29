// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string>
#include <vector>

struct Resource {
  std::string mName;
  std::string mValue;
  std::string mType;
  bool mIsAlias {false};
  std::vector<std::string> mDependencies;
};