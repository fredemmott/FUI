// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/NativeWaitable.hpp>
#include <algorithm>
#include <chrono>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace FredEmmott::GUI {

struct FrameRateRequirement {
  struct SmoothAnimation {};
  struct After {
    std::chrono::steady_clock::time_point mValue;
  };

  constexpr FrameRateRequirement() = default;
  constexpr FrameRateRequirement(SmoothAnimation) noexcept
    : mSmoothAnimation(true) {}
  FrameRateRequirement(const After after) noexcept : mAfter(after.mValue) {}

  FrameRateRequirement(FrameRateRequirement&&) noexcept = default;
  FrameRateRequirement(const NativeWaitable& e) noexcept {
    mNativeWaitables.emplace_back(e);
  }

  FrameRateRequirement(
    std::initializer_list<NativeWaitable> nativeEvents) noexcept {
    mNativeWaitables.append_range(nativeEvents);
    std::ranges::sort(mNativeWaitables);
    const auto [first, last] = std::ranges::unique(mNativeWaitables);
    mNativeWaitables.erase(first, last);
  }

  template <std::ranges::input_range R>
    requires std::same_as<
      std::remove_cvref_t<std::ranges::range_value_t<R>>,
      FrameRateRequirement>
  FrameRateRequirement(R&& requirements) noexcept {
    for (const FrameRateRequirement& other: requirements) {
      Merge(other);
    }
  }

  [[nodiscard]]
  bool RequiresSmoothAnimation() const noexcept {
    return mSmoothAnimation;
  }

  [[nodiscard]]
  std::optional<std::chrono::steady_clock::time_point> GetAfter()
    const noexcept {
    return mAfter;
  }

  [[nodiscard]]
  std::span<const NativeWaitable> GetNativeWaitables() const noexcept {
    return mNativeWaitables;
  }

 private:
  bool mSmoothAnimation {false};
  std::optional<std::chrono::steady_clock::time_point> mAfter;
  // Always sorted
  std::vector<NativeWaitable> mNativeWaitables;

  // poor man's flat_set (not in MSVC 2022)
  template <class T>
    requires std::same_as<std::remove_cvref_t<T>, std::vector<NativeWaitable>>
  void MergeNativeWaitables(T&& other) noexcept {
    if (other.empty()) {
      return;
    }
    if (mNativeWaitables.empty()) {
      mNativeWaitables = std::forward<T>(other);
      return;
    }

    std::vector<NativeWaitable> merged;
    merged.reserve(mNativeWaitables.size() + other.size());
    std::ranges::set_union(
      mNativeWaitables, std::forward<T>(other), std::back_inserter(merged));
    mNativeWaitables = std::move(merged);
  }

  template <class T>
    requires std::same_as<std::remove_cvref_t<T>, FrameRateRequirement>
  void Merge(T&& other) {
    if (other.mSmoothAnimation) {
      mSmoothAnimation = true;
    }
    if (other.mAfter && ((!mAfter) || *other.mAfter < *mAfter)) {
      mAfter = other.mAfter;
    }
    MergeNativeWaitables(std::forward_like<T>(other.mNativeWaitables));
  }
};

}// namespace FredEmmott::GUI
