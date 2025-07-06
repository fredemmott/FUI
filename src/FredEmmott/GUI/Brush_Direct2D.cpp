// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Brush.hpp"
#include "Direct2DRenderer.hpp"
#include "detail/win32_detail.hpp"

namespace FredEmmott::GUI {

template <>
wil::com_ptr<ID2D1Brush> Brush::as<wil::com_ptr<ID2D1Brush>>(
  Renderer* renderer,
  const Rect& rect) const {
  if (mD2DSolidColorBrush) {
    return mD2DSolidColorBrush;
  }

  const auto rt = direct2d_device_context_cast(renderer);

  if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
    win32_detail::CheckHResult(rt->CreateSolidColorBrush(
      it->as<D2D1_COLOR_F>(), mD2DSolidColorBrush.put()));
    return mD2DSolidColorBrush;
  }
  if (const auto it = get_if<LinearGradientBrush>(&mBrush)) {
    return it->GetDirect2DBrush(rt, rect);
  }

  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI
