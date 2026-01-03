// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <algorithm>
#include <chrono>
#include <span>

namespace FredEmmott::GUI {

struct FrameRateRequirement {
  struct SmoothAnimation {};
  struct After {
    std::chrono::steady_clock::time_point mValue;
  };
  struct NativeEvent {
#ifdef _WIN32
    void* mHandle {nullptr};
#else
    int mFD {-1};
#endif
    constexpr auto operator<=>(const NativeEvent&) const = default;
  };

  constexpr FrameRateRequirement() = default;
  constexpr FrameRateRequirement(SmoothAnimation) noexcept
    : mSmoothAnimation(true) {}
  constexpr FrameRateRequirement(const After after) noexcept {
    if ((!mAfter) || after.mValue < *mAfter) {
      mAfter = after.mValue;
    }
  }

  constexpr FrameRateRequirement(
    std::initializer_list<NativeEvent> nativeEvents) noexcept {
    mNativeEvents.append_range(nativeEvents);
    std::ranges::sort(mNativeEvents);
    const auto [first, last] = std::ranges::unique(mNativeEvents);
    mNativeEvents.erase(first, last);
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
  std::span<const NativeEvent> GetNativeEvents() const noexcept {
    return mNativeEvents;
  }

 private:
  bool mSmoothAnimation {false};
  std::optional<std::chrono::steady_clock::time_point> mAfter;
  // Always sorted
  std::vector<NativeEvent> mNativeEvents;

  // poor man's flat_set (not in MSVC 2022)
  void MergeNativeEvents(const FrameRateRequirement& other) noexcept {
    const auto oldSize = mNativeEvents.end() - mNativeEvents.begin();
    mNativeEvents.append_range(other.mNativeEvents);
    auto newItems = std::ranges::subrange(
      mNativeEvents.begin() + oldSize, mNativeEvents.end());

    std::ranges::inplace_merge(mNativeEvents, mNativeEvents.begin() + oldSize);
    const auto [first, last] = std::ranges::unique(mNativeEvents);
    mNativeEvents.erase(first, last);
  }

  void Merge(const FrameRateRequirement& other) {
    if (other.mSmoothAnimation) {
      mSmoothAnimation = true;
    }
    if (other.mAfter && ((!mAfter) || *other.mAfter < *mAfter)) {
      mAfter = other.mAfter;
    }
    MergeNativeEvents(other);
  }
};

}// namespace FredEmmott::GUI
