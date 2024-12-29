// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SortResources.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <unordered_set>

struct SortState {
  std::vector<Resource> mPending;
  std::unordered_set<std::string> mPendingKeys;
};

void SortResourcesIteration(
  std::back_insert_iterator<std::vector<Resource>>& inserter,
  SortState* state) {
  auto& [resources, keys] = *state;

  // Remove resolved or unknown dependencies
  for (auto&& resource: resources) {
    auto it = resource.mDependencies.begin();
    while (it != resource.mDependencies.end()) {
      if ((*it == resource.mName) || !keys.contains(*it)) {
        it = resource.mDependencies.erase(it);
      } else {
        ++it;
      }
    }
  }

  auto it = resources.begin();
  while (it != resources.end()) {
    if (it->mDependencies.empty()) {
      state->mPendingKeys.erase(it->mName);
      inserter = std::move(*it);
      it = resources.erase(it);
    } else {
      ++it;
    }
  }
}

void SortResources(std::vector<Resource>& inout) {
  SortState state {
    .mPending = inout,
    .mPendingKeys = std::views::transform(inout, &Resource::mName)
      | std::ranges::to<std::unordered_set>(),
  };
  inout.clear();

  // Sort by name first, then by dependencies
  std::ranges::sort(state.mPending, {}, &Resource::mName);

  // Okay, dependencies :)
  auto inserter = std::back_inserter(inout);
  std::size_t count = 0;
  while (!state.mPending.empty()) {
    if (++count > 10) {
      throw std::runtime_error("SortResources: dependency loop");
    }
    SortResourcesIteration(inserter, &state);
  }
}