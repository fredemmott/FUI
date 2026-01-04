// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/NativeWaitable.hpp>
#include <boost/container/flat_set.hpp>
#include <chrono>
#include <optional>
#include <span>
#include <utility>

namespace FredEmmott::GUI {

struct FrameRateRequirement {
  struct SmoothAnimation {};
  struct After {
    std::chrono::steady_clock::time_point mValue;
  };

  constexpr FrameRateRequirement() = default;
  constexpr FrameRateRequirement(FrameRateRequirement&&) noexcept = default;
  constexpr FrameRateRequirement(SmoothAnimation) noexcept
    : mSmoothAnimation(true) {}

  FrameRateRequirement(const After after) noexcept : mAfter(after.mValue) {}

  FrameRateRequirement(const NativeWaitable& w) noexcept
    : mNativeWaitables {std::in_place, {w}} {}

  FrameRateRequirement(
    std::initializer_list<NativeWaitable> nativeEvents) noexcept
    : mNativeWaitables(std::in_place, nativeEvents) {}

  template <std::ranges::input_range R>
    requires std::same_as<
      std::remove_cvref_t<std::ranges::range_value_t<R>>,
      FrameRateRequirement>
  constexpr FrameRateRequirement(R&& requirements) noexcept {
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
    if (mNativeWaitables) {
      return *mNativeWaitables;
    }
    return {};
  }

 private:
  bool mSmoothAnimation {false};
  std::optional<std::chrono::steady_clock::time_point> mAfter;
  // - Using boost because `std::flat_set` is not available in MSVC 2022
  //   as of 2026-01-04
  // - Using `std::optional` because we *really* want the default and
  //  `SmoothAnimation` constructors to be constexpr; this would need
  //   C++26's `__cpp_lib_constexpr_flat_set`
  std::optional<boost::container::flat_set<NativeWaitable>> mNativeWaitables;

  template <class T>
    requires std::same_as<std::remove_cvref_t<T>, FrameRateRequirement>
  constexpr void Merge(T&& other) {
    if (other.mSmoothAnimation) {
      mSmoothAnimation = true;
    }
    if (other.mAfter && ((!mAfter) || *other.mAfter < *mAfter)) {
      mAfter = other.mAfter;
    }

    if (!other.mNativeWaitables) {
      return;
    }
    if (!mNativeWaitables) {
      mNativeWaitables = std::forward<T>(other).mNativeWaitables;
    } else {
      mNativeWaitables->insert(
        boost::container::ordered_unique_range,
        other.mNativeWaitables->begin(),
        other.mNativeWaitables->end());
    }
  }
};

}// namespace FredEmmott::GUI
