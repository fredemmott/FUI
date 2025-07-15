// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <unordered_map>

namespace FredEmmott::utility {

static_assert(
  GUI::Config::CompilerChecks::MinimumCPlusPlus < 202600
    || !GUI::Config::LibraryDeveloper,
  "Consider replacing with `std::unordered_map` (P3372)");

/* Like an `std::unordered_map()` but has a constexpr empty constructor.
 *
 * This primarily exists for the `GUI::Style` class
 */
template <class K, class V>
struct unordered_map {
  using map_type = std::unordered_map<K, V>;
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const K, V>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using hasher = std::hash<K>;
  using key_equal = std::equal_to<K>;
  using allocator_type = std::allocator<std::pair<const K, V>>;
  using reference = value_type&;
  using iterator_ = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;
  using iterator = std::conditional_t<
    std::is_const_v<std::remove_reference_t<K>>,
    const_iterator,
    iterator_>;

  constexpr unordered_map() = default;
  unordered_map(std::initializer_list<value_type> init)
    : mData(std::make_unique<map_type>(init)) {}
  constexpr unordered_map(const unordered_map& other) noexcept {
    if (!other.mData) {
      return;
    }
    mData = std::make_unique<map_type>(*other.mData);
  }
  constexpr unordered_map& operator=(const unordered_map& other) noexcept {
    mData.reset();
    if (other.mData) {
      mData = std::make_unique<map_type>(*other.mData);
    }
    return *this;
  }
  constexpr unordered_map(unordered_map&&) noexcept = default;
  constexpr unordered_map& operator=(unordered_map&&) noexcept = default;

  constexpr auto begin() const {
    if (mData) {
      return mData->begin();
    }
    return decltype(mData->begin()) {};
  }

  constexpr auto end() const {
    if (mData) {
      return mData->end();
    }
    return decltype(mData->end()) {};
  }

  constexpr bool contains(K k) const noexcept {
    if (!mData) {
      return false;
    }
    return mData->contains(k);
  }

  constexpr bool empty() const noexcept {
    if (!mData) {
      return true;
    }
    return mData->empty();
  }

  constexpr bool operator==(const unordered_map& other) const noexcept {
    if (empty() != other.empty()) {
      return false;
    }
    if (empty()) {
      return true;
    }
    return (*mData == *other.mData);
  }

  void clear() noexcept {
    mData.reset();
  }

  template <std::convertible_to<K> T>
  auto& operator[](T&& key) {
    if (!mData) {
      mData = std::make_unique<map_type>();
    }
    return mData->operator[](std::forward<T>(key));
  }

  template <std::convertible_to<K> T>
  const auto& at(T&& key) const {
    if (!mData) {
      throw std::out_of_range("Container is empty for const at()");
    }
    return mData->at(std::forward<T>(key));
  }

  template <std::convertible_to<K> T>
  auto& at(T&& key) {
    if (!mData) {
      throw std::out_of_range("Container is empty for at()");
    }
    return mData->at(std::forward<T>(key));
  }

  template <std::convertible_to<K> T>
  const auto& operator[](T&& key) const {
    return at(std::forward<T>(key));
  }

  template <class... Args>
  auto insert_or_assign(Args&&... args) {
    if (!mData) {
      mData = std::make_unique<map_type>();
    }
    return mData->insert_or_assign(std::forward<Args>(args)...);
  }

  template <class... Args>
  auto emplace(Args&&... args) {
    if (!mData) {
      mData = std::make_unique<map_type>();
    }
    return mData->emplace(std::forward<Args>(args)...);
  }

  template <std::convertible_to<K> U = K>
  void erase(U&& k) {
    if (!mData) {
      return;
    }
    mData->erase(std::forward<U>(k));
  }

 private:
  std::unique_ptr<map_type> mData;
};
}// namespace FredEmmott::utility