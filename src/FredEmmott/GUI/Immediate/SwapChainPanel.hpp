// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/SwapChainPanel.hpp>

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct SwapChainPanelResultMixin {
  [[nodiscard]]
  auto GetSwapChain(this auto&& self) {
    return widget_from_result<Widgets::SwapChainPanel>(self)->GetSwapChain();
  }

  template <class Self>
  decltype(auto) GetSwapChain(
    this Self&& self,
    Widgets::SwapChainPanel::SwapChain* p) {
    FUI_ASSERT(p);
    *p = widget_from_result<Widgets::SwapChainPanel>(self)->GetSwapChain();
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) GetSwapChain(
    this Self&& self,
    std::invocable<Widgets::SwapChainPanel::SwapChain> auto&& fn) {
    std::invoke(
      std::forward<decltype(fn)>(fn),
      widget_from_result<Widgets::SwapChainPanel>(self)->GetSwapChain());
    return std::forward<Self>(self);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail

namespace FredEmmott::GUI::Immediate {

using SwapChainPanelResult = Result<
  nullptr,
  void,
  immediate_detail::SwapChainPanelResultMixin,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin>;

[[nodiscard]]
inline SwapChainPanelResult SwapChainPanel(
  const ID id = ID {std::source_location::current()}) {
  return immediate_detail::ChildlessWidget<Widgets::SwapChainPanel>(id);
}

template <class T>
  requires requires(SwapChainPanelResult r, std::remove_cvref_t<T> v) {
    std::ignore = r.GetSwapChain(v);
  }
inline SwapChainPanelResult SwapChainPanel(
  T&& swapChainReceiver,
  const ID id = ID {std::source_location::current()}) {
  return SwapChainPanel(id).GetSwapChain(std::forward<T>(swapChainReceiver));
}

}// namespace FredEmmott::GUI::Immediate
