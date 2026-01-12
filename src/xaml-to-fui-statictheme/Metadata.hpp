// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string>
#include <vector>

struct Metadata {
  std::string mComponent;
  std::string mParent;
  std::string mNamespace;
  std::string mDetailNamespace;
  std::vector<std::string> mCppUsesNamespaces;
};