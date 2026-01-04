// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <algorithm>
#include <chrono>
#include <optional>
#include <span>
#include <vector>

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
  FrameRateRequirement(const After after) noexcept : mAfter(after.mValue) {}

  FrameRateRequirement(FrameRateRequirement&&) noexcept = default;
  FrameRateRequirement(const NativeEvent& e) noexcept {
    mNativeEvents.emplace_back(e);
  }

  FrameRateRequirement(
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
  template <class T>
    requires std::same_as<std::remove_cvref_t<T>, std::vector<NativeEvent>>
  void MergeNativeEvents(T&& other) noexcept {
    if (other.empty()) {
      return;
    }
    if (mNativeEvents.empty()) {
      mNativeEvents = std::forward<T>(other);
      return;
    }

    std::vector<NativeEvent> merged;
    merged.reserve(mNativeEvents.size() + other.size());
    std::ranges::set_union(
      mNativeEvents, std::forward<T>(other), std::back_inserter(merged));
    mNativeEvents = std::move(merged);
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
    MergeNativeEvents(std::forward_like<T>(other.mNativeEvents));
  }
};

}// namespace FredEmmott::GUI
