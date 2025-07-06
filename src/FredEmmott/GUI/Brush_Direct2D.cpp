// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Brush.hpp"
#include "detail/win32_detail.hpp"

namespace FredEmmott::GUI {

wil::com_ptr<ID2D1Brush> Brush::GetDirect2DBrush(
  ID2D1RenderTarget* rt,
  const Rect& rect) const {
  if (mD2DSolidColorBrush) {
    return mD2DSolidColorBrush;
  }

  if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
    win32_detail::CheckHResult(rt->CreateSolidColorBrush(
      it->as<D2D1_COLOR_F>(), mD2DSolidColorBrush.put()));
    return mD2DSolidColorBrush;
  }
  if (const auto it = get_if<LinearGradientBrush>(&mBrush)) {
    return it->GetDirect2DBrush(rt, rect);
  }
  if (const auto it = get_if<StaticThemeBrush>(&mBrush)) {
    return (*it)->Resolve()->GetDirect2DBrush(rt, rect);
  }

  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI
