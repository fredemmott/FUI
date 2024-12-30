// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <unordered_set>

struct Resource {
  enum class Kind {
    Any,
    Alias,
    Literal,
  };
  std::string mName;
  std::string mValue;
  std::string mType;
  std::unordered_set<std::string> mDependencies;
  Kind mKind {Kind::Any};

  constexpr bool IsAlias() const {
    return mKind == Kind::Alias;
  }

  constexpr bool IsLiteral() const {
    return mKind == Kind::Literal;
  }
};