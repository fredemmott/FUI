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

template <std::derived_from<Widgets::Widget> T = Widgets::Widget, class TResult>
  requires requires { widget_from_result_t<TResult> {}; }
T* widget_from_result(const TResult& v) {
  return dynamic_cast<T*>(widget_from_result_t<TResult> {}(v));
}
}// namespace FredEmmott::GUI::Immediate::immediate_detail