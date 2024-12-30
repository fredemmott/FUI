// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <unordered_set>

struct Resource {
  std::string mName;
  std::string mValue;
  std::string mType;
  bool mIsAlias {false};
  std::unordered_set<std::string> mDependencies;
};