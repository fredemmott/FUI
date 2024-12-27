// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>
#include <vector>
#include <ranges>
#include <span>

namespace FredEmmott::GUI::Widgets {

class Widget;

struct WidgetList {
  using TSpan = std::span<Widget* const>;
  using TVector = std::vector<Widget*>;

  WidgetList() = delete;
  explicit WidgetList(TSpan span) : mStorage {span} {
  }

  WidgetList(TVector&& vec) : mStorage(std::move(vec)) {
  }

  WidgetList(std::initializer_list<Widget*> list) : mStorage(TVector {list}) {
  }

  [[nodiscard]]
  auto begin() const {
    return Get().begin();
  }

  [[nodiscard]]
  auto end() const {
    return Get().end();
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return Get().empty();
  }

 private:
  mutable std::variant<TSpan, TVector> mStorage;

  [[nodiscard]]
  std::span<Widget* const> Get() const {
    if (const auto span = get_if<TSpan>(&mStorage)) {
      return *span;
    }
    if (const auto vec = get_if<TVector>(&mStorage)) {
      return std::span {*vec};
    }
    throw std::bad_variant_access {};
  }
};
static_assert(std::ranges::input_range<WidgetList>);

}