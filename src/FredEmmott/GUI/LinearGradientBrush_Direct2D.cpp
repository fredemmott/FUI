// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "LinearGradientBrush.hpp"
#include "detail/win32_detail.hpp"

using namespace FredEmmott::GUI::win32_detail;

namespace FredEmmott::GUI {

wil::com_ptr<ID2D1Brush> LinearGradientBrush::GetDirect2DBrush(
  ID2D1RenderTarget* rt,
  const Rect& rect) const {
  const_cast<LinearGradientBrush*>(this)->InitializeDirect2DBrush(rt);

  auto m = D2D1::Matrix3x2F::Identity();
  if (mMappingMode == MappingMode::RelativeToBoundingBox) {
    m = D2D1::Matrix3x2F::Scale(rect.GetWidth(), rect.GetHeight());
  }
  m = m * D2D1::Matrix3x2F::Translation(rect.GetLeft(), rect.GetTop());

  auto brush = mDirect2DCache->mDirect2DBrush;
  brush->SetTransform(m);
  return brush;
}

void LinearGradientBrush::InitializeDirect2DBrush(ID2D1RenderTarget* rt) {
  if (mDirect2DCache) {
    return;
  }
  mDirect2DCache = std::make_shared<Direct2DCache>();
  auto& [d2dStops, d2dBrush] = *mDirect2DCache;

  std::vector<D2D1_GRADIENT_STOP> stops;
  stops.reserve(mStops.size());
  for (auto&& stop: mStops) {
    stops.emplace_back(stop.mOffset, stop.mColor.as<D2D1_COLOR_F>());
  }

  CheckHResult(rt->CreateGradientStopCollection(
    stops.data(),
    stops.size(),
    D2D1_GAMMA_2_2,
    D2D1_EXTEND_MODE_CLAMP,
    d2dStops.put()));
  CheckHResult(rt->CreateLinearGradientBrush(
    {
      mStart.as<D2D1_POINT_2F>(),
      mEnd.as<D2D1_POINT_2F>(),
    },
    d2dStops.get(),
    d2dBrush.put()));
}

}// namespace FredEmmott::GUI