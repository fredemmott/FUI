// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Brush.hpp"
#include "Direct2DRenderer.hpp"
#include "assert.hpp"
#include "detail/win32_detail.hpp"

namespace FredEmmott::GUI {

using namespace win32_detail;

namespace {
template <class T>
concept as_color = requires(T& t) { t.template as<D2D1_COLOR_F>(); };
}// namespace

template <>
ID2D1Brush* Brush::as<ID2D1Brush*>(Renderer* renderer, const Rect& rect) const {
  if (mD2DBrush && *mD2DBrush) {
    return mD2DBrush->get();
  }

  const auto rt = direct2d_device_context_cast(renderer);

  mD2DBrush.reset();
  std::visit(
    felly::overload {
      [rt, this](const as_color auto& brush) {
        wil::com_ptr<ID2D1SolidColorBrush> solidColorBrush;
        CheckHResult(rt->CreateSolidColorBrush(
          brush.template as<D2D1_COLOR_F>(), solidColorBrush.put()));
        mD2DBrush.emplace(std::move(solidColorBrush));
      },
      [rt, rect, this](const LinearGradientBrush& brush) {
        mD2DBrush.emplace(brush.GetDirect2DBrush(rt, rect));
      },
    },
    mBrush);
  FUI_ASSERT(mD2DBrush && *mD2DBrush);
  return mD2DBrush->get();
}

}// namespace FredEmmott::GUI
