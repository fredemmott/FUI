// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
namespace FredEmmott::GUI::Immediate::immediate_detail {
template <class T>
struct widget_from_result_t {};

template <class T>
  requires requires(T& v) { T::HasWidget; } && T::HasWidget
struct widget_from_result_t<T> {
  static auto operator()(const T& v) {
    return v.mWidget;
  };
};

template <class T>
  requires requires { widget_from_result_t<T> {}; }
auto widget_from_result(const T& v) {
  return widget_from_result_t<T> {}(v);
}
}// namespace FredEmmott::GUI::Immediate::immediate_detail