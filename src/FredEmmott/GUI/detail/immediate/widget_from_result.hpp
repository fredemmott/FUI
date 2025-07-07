// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct widget_from_result_t {
  static Widgets::Widget* GetWidget(const auto& v) {
    return v.mWidget;
  };
};

template <std::derived_from<Widgets::Widget> T = Widgets::Widget>
[[nodiscard]]
T* widget_from_result(const auto& v) {
  const auto w = widget_from_result_t::GetWidget(v);
  if constexpr (std::same_as<T, Widgets::Widget>) {
    return w;
  } else {
    return dynamic_cast<T*>(w);
  }
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail