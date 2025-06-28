// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Direct2DRenderer.hpp"

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Font.hpp>
#include <FredEmmott/GUI/Rect.hpp>

#include "assert.hpp"
#include "detail/direct_write_detail/DirectWriteFontProvider.hpp"
#include "detail/win32_detail.hpp"

using namespace FredEmmott::GUI::direct_write_detail;
using namespace FredEmmott::GUI::win32_detail;
using namespace FredEmmott::GUI::font_detail;

namespace FredEmmott::GUI {
Direct2DRenderer::Direct2DRenderer(ID2D1DeviceContext* deviceContext)
  : mDeviceContext(deviceContext) {}

Direct2DRenderer::~Direct2DRenderer() {
  FUI_ASSERT(mStateStack.empty());
}

void Direct2DRenderer::PushLayer(float alpha) {
  if (std::abs(alpha - 1.0f) <= std::numeric_limits<float>::epsilon()) {
    D2D1_MATRIX_3X2_F t {};
    mDeviceContext->GetTransform(&t);
    mStateStack.push(t);
    return;
  }

  const D2D1_LAYER_PARAMETERS1 params {
    .contentBounds = D2D1::InfiniteRect(),
    .maskAntialiasMode = D2D1_ANTIALIAS_MODE_ALIASED,
    .opacity = alpha,
  };

  mDeviceContext->PushLayer(&params, nullptr);
  mStateStack.push(NativeLayer {});
}

void Direct2DRenderer::PopLayer() {
  if (const auto t = get_if<D2D1_MATRIX_3X2_F>(&mStateStack.top()); t) {
    mDeviceContext->SetTransform(*t);
    mStateStack.pop();
    return;
  }
  if (holds_alternative<NativeLayer>(mStateStack.top())) {
    mStateStack.pop();
    mDeviceContext->PopLayer();
    return;
  }
  throw std::logic_error("Invalid state stack");
}

void Direct2DRenderer::PushClipRect(const Rect& rect) {
  mDeviceContext->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_ALIASED);
}

void Direct2DRenderer::PopClipRect() {
  mDeviceContext->PopAxisAlignedClip();
}

void Direct2DRenderer::Scale(float x, float y) {
  if (
    std::abs(x - 1.0f) <= std::numeric_limits<float>::epsilon()
    && std::abs(y - 1.0f) <= std::numeric_limits<float>::epsilon()) {
    return;
  }
  this->PostTransform(D2D1::Matrix3x2F::Scale(x, y));
}

void Direct2DRenderer::Translate(const Point& point) {
  if (
    std::max(point.mX, point.mY) < std::numeric_limits<float>::epsilon()
    && std::min(point.mX, point.mY) > -std::numeric_limits<float>::epsilon()) {
    return;
  }
  this->PostTransform(D2D1::Matrix3x2F::Translation(point.mX, point.mY));
}

void Direct2DRenderer::FillRect(const Brush& brush, const Rect& rect) {
  mDeviceContext->FillRectangle(
    rect, brush.GetDirect2DBrush(mDeviceContext, rect).get());
}

void Direct2DRenderer::StrokeRect(
  const Brush& brush,
  const Rect& rect,
  const float thickness) {
  mDeviceContext->DrawRectangle(
    rect,
    brush.GetDirect2DBrush(mDeviceContext, rect).get(),
    thickness == 0 ? 1 : thickness,
    nullptr);
}

void Direct2DRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius) {
  mDeviceContext->FillRoundedRectangle(
    {rect, radius, radius}, brush.GetDirect2DBrush(mDeviceContext, rect).get());
}

void Direct2DRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius,
  float thickness) {
  mDeviceContext->DrawRoundedRectangle(
    {rect, radius, radius},
    brush.GetDirect2DBrush(mDeviceContext, rect).get(),
    thickness == 0 ? 1 : thickness,
    nullptr);
}

void Direct2DRenderer::DrawText(
  const Brush& brush,
  const Rect& brushRect,
  const Font& font,
  std::string_view text,
  const Point& baseline) {
  if (text.empty()) {
    return;
  }
  const auto wideText = Utf8ToWide(text);
  const auto props = font.as<DirectWriteFont>();
  const auto tf = props.mTextFormat.get();
  mDeviceContext->DrawText(
    wideText.data(),
    wideText.size(),
    tf,
    Rect {
      baseline,
      {std::numeric_limits<float>::infinity(), font.GetMetrics().mAscent}},
    brush.GetDirect2DBrush(mDeviceContext, brushRect).get(),
    D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
    DWRITE_MEASURING_MODE_NATURAL);
}
void Direct2DRenderer::PostTransform(const D2D1_MATRIX_3X2_F& transform) {
  D2D1_MATRIX_3X2_F combined {};
  mDeviceContext->GetTransform(&combined);
  combined = transform * combined;
  mDeviceContext->SetTransform(combined);
}

void Direct2DRenderer::Clear(const Color& color) {
  mDeviceContext->Clear(color.as<D2D1_COLOR_F>());
}

}// namespace FredEmmott::GUI
