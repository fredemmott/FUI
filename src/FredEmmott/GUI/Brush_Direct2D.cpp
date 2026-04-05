// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Brush.hpp"
#include "Direct2DRenderer.hpp"
#include "assert.hpp"
#include "detail/win32_detail.hpp"

namespace FredEmmott::GUI {

using namespace win32_detail;

namespace {

ID2D1Brush* GetSolidColorBrush(
  ID2D1DeviceContext* rt,
  const Color::Constant& color) {
  thread_local std::unordered_map<uint32_t, wil::com_ptr<ID2D1Brush>> cache {};
  const auto key = color.ToBGRA32();
  auto& it = cache[key];
  if (it) {
    return it.get();
  }
  wil::com_ptr<ID2D1SolidColorBrush> solidColorBrush;
  CheckHResult(
    rt->CreateSolidColorBrush(color.as<D2D1_COLOR_F>(), solidColorBrush.put()));
  it = std::move(solidColorBrush);
  return it.get();
}

}// namespace

template <>
ID2D1Brush* Brush::as<ID2D1Brush*>(Renderer* renderer, const Rect& rect) const {
  const auto rt = direct2d_device_context_cast(renderer);
  return std::visit(
    felly::overload {
      [rt](const SolidColorBrush& brush) {
        return GetSolidColorBrush(rt, brush.Resolve());
      },
      [rt](const AcrylicBrush& brush) {
        return GetSolidColorBrush(rt, brush.Resolve());
      },
      [rt, rect](const LinearGradientBrush& brush) {
        return brush.GetDirect2DBrush(rt, rect);
      },
    },
    mBrush);
}

}// namespace FredEmmott::GUI
