// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Direct2DRenderer.hpp"

#include <d3d11.h>

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Font.hpp>
#include <FredEmmott/GUI/Rect.hpp>
#include <felly/overload.hpp>
#include <felly/scope_exit.hpp>

#include "assert.hpp"
#include "detail/direct_write_detail/DirectWriteFontProvider.hpp"
#include "detail/win32_detail.hpp"

using namespace FredEmmott::GUI::direct_write_detail;
using namespace FredEmmott::GUI::win32_detail;
using namespace FredEmmott::GUI::font_detail;

namespace FredEmmott::GUI {

namespace {

struct ImportedDirect2DTexture : ImportedTexture {
  ~ImportedDirect2DTexture() override = default;

  wil::com_ptr<ID2D1Bitmap1> mBitmap;
};

struct ImportedDirect3DFence : ImportedFence {
  ~ImportedDirect3DFence() override = default;
  wil::com_ptr<ID3D11Fence> mFence;
};

}// namespace

Direct2DRenderer::Direct2DRenderer(
  ID3D11Device5* device,
  ID2D1DeviceContext* deviceContext)
  : mD3DDevice(device),
    mDeviceContext(deviceContext) {
  wil::com_ptr<ID3D11DeviceContext3> dc3;
  mD3DDevice->GetImmediateContext3(dc3.put());
  // ID3D11DeviceContext4 for Wait(fence, value)
  dc3.query_to(mD3DDeviceContext.put());
}

Direct2DRenderer::~Direct2DRenderer() {
  FUI_ASSERT(mStateStack.empty());
}

void Direct2DRenderer::PushLayer(const float alpha) {
  mStateStack.emplace();
  auto& state = mStateStack.top();
  mDeviceContext->GetTransform(&state.mTransform);

  // 99.9% opaque is close enough to fully opaque
  const bool isFullyOpaque = std::abs(alpha - 1.0f) < 0.001;
  if (isFullyOpaque) {
    return;
  }

  const D2D1_LAYER_PARAMETERS1 params {
    .contentBounds = D2D1::InfiniteRect(),
    .maskAntialiasMode = D2D1_ANTIALIAS_MODE_ALIASED,
    .opacity = alpha,
  };

  mDeviceContext->PushLayer(&params, nullptr);
  state.mHaveNativeLayer = true;
}

void Direct2DRenderer::PopLayer() {
  const auto pop = felly::scope_exit([this] { mStateStack.pop(); });

  auto& state = mStateStack.top();
  if (state.mHaveNativeLayer) {
    mDeviceContext->PopLayer();
  }
  mDeviceContext->SetTransform(state.mTransform);
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
    rect, brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get());
}

void Direct2DRenderer::StrokeRect(
  const Brush& brush,
  const Rect& rect,
  const float thickness) {
  mDeviceContext->DrawRectangle(
    rect,
    brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get(),
    thickness == 0 ? 1 : thickness,
    nullptr);
}
void Direct2DRenderer::DrawLine(
  const Brush& brush,
  const Point& start,
  const Point& end,
  const float thickness) {
  mDeviceContext->DrawLine(
    start.as<D2D1_POINT_2F>(),
    end.as<D2D1_POINT_2F>(),
    brush.as<wil::com_ptr<ID2D1Brush>>(this, {start, end}).get(),
    thickness == 0 ? 1 : thickness,
    nullptr);
}

void Direct2DRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius) {
  mDeviceContext->FillRoundedRectangle(
    {rect, radius, radius},
    brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get());
}
void Direct2DRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float topLeftRadius,
  float topRightRadius,
  [[maybe_unused]] float bottomRightRadius,
  [[maybe_unused]] float bottomLeftRadius) {
  wil::com_ptr<ID2D1PathGeometry> path;
  wil::com_ptr<ID2D1Factory> factory;
  mDeviceContext->GetFactory(&factory);
  CheckHResult(factory->CreatePathGeometry(&path));
  wil::com_ptr<ID2D1GeometrySink> sink;
  CheckHResult(path->Open(&sink));
  sink->BeginFigure(
    (rect.GetTopLeft() - Point {0, -topLeftRadius}).as<D2D1_POINT_2F>(),
    D2D1_FIGURE_BEGIN_FILLED);
  sink->AddArc({
    (rect.GetTopLeft() + Point {topLeftRadius, 0}).as<D2D1_POINT_2F>(),
    {topLeftRadius, topLeftRadius},
    90,
    D2D1_SWEEP_DIRECTION_CLOCKWISE,
    D2D1_ARC_SIZE_SMALL,
  });
  sink->AddLine(
    (rect.GetTopRight() - Point {topRightRadius, 0}).as<D2D1_POINT_2F>());
  sink->AddArc({
    (rect.GetTopRight() + Point {0, topRightRadius}).as<D2D1_POINT_2F>(),
    {topRightRadius, topRightRadius},
    90,
    D2D1_SWEEP_DIRECTION_CLOCKWISE,
    D2D1_ARC_SIZE_SMALL,
  });
  sink->AddLine(
    (rect.GetBottomRight() - Point {0, bottomRightRadius}).as<D2D1_POINT_2F>());
  sink->AddArc({
    (rect.GetBottomRight() - Point {bottomRightRadius, 0}).as<D2D1_POINT_2F>(),
    {bottomRightRadius, bottomRightRadius},
    90,
    D2D1_SWEEP_DIRECTION_CLOCKWISE,
    D2D1_ARC_SIZE_SMALL,
  });
  sink->AddLine(
    (rect.GetBottomLeft() + Point {bottomLeftRadius, 0}).as<D2D1_POINT_2F>());
  sink->AddArc({
    (rect.GetBottomLeft() - Point {0, bottomLeftRadius}).as<D2D1_POINT_2F>(),
    {bottomLeftRadius, bottomLeftRadius},
    90,
    D2D1_SWEEP_DIRECTION_CLOCKWISE,
    D2D1_ARC_SIZE_SMALL,
  });
  sink->EndFigure(D2D1_FIGURE_END_CLOSED);
  CheckHResult(sink->Close());
  mDeviceContext->FillGeometry(
    path.get(), brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get(), nullptr);
}

void Direct2DRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius,
  float thickness) {
  mDeviceContext->DrawRoundedRectangle(
    {rect, radius, radius},
    brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get(),
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
      Size {std::numeric_limits<float>::infinity(), font.GetMetrics().mAscent}},
    brush.as<wil::com_ptr<ID2D1Brush>>(this, brushRect).get(),
    D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
    DWRITE_MEASURING_MODE_NATURAL);
}

std::unique_ptr<ImportedTexture> Direct2DRenderer::ImportTexture(
  HANDLE const handle,
  const TextureHandleKind kind) const {
  wil::com_ptr<ID3D11Texture2D> texture;
  switch (kind) {
    case TextureHandleKind::LegacySharedHandle:
      CheckHResult(mD3DDevice->OpenSharedResource(
        handle, __uuidof(ID3D11Texture2D), texture.put_void()));
      break;
    case TextureHandleKind::NTHandle:
      CheckHResult(mD3DDevice->OpenSharedResource1(
        handle, __uuidof(ID3D11Texture2D), texture.put_void()));
      break;
  }
  const auto surface = texture.query<IDXGISurface>();
  wil::com_ptr<ID2D1Bitmap1> bitmap;
  CheckHResult(mDeviceContext->CreateBitmapFromDxgiSurface(
    surface.get(), nullptr, bitmap.put()));
  auto ret = std::make_unique<ImportedDirect2DTexture>();
  ret->mBitmap = std::move(bitmap);
  return ret;
}

std::unique_ptr<ImportedFence> Direct2DRenderer::ImportFence(
  HANDLE const handle) const {
  wil::com_ptr<ID3D11Fence> fence;
  CheckHResult(mD3DDevice->OpenSharedFence(handle, IID_PPV_ARGS(fence.put())));
  auto ret = std::make_unique<ImportedDirect3DFence>();
  ret->mFence = std::move(fence);
  return ret;
}

void Direct2DRenderer::DrawTexture(
  const Rect& sourceRect,
  const Rect& destRect,
  ImportedTexture* const rawTexture,
  ImportedFence* const rawFence,
  uint64_t fenceValue) {
#ifndef NDEBUG
#define IMPL_CAST dynamic_cast
#else
#define IMPL_CAST static_cast
#endif
  FUI_ASSERT(rawTexture);
  FUI_ASSERT(rawFence);
  FUI_ASSERT(fenceValue > 0, "A wait for fence 0 always succeeds");
  const auto texture
    = IMPL_CAST<ImportedDirect2DTexture*>(rawTexture)->mBitmap.get();
  const auto fence = IMPL_CAST<ImportedDirect3DFence*>(rawFence)->mFence.get();
  CheckHResult(mD3DDeviceContext->Wait(fence, fenceValue));
  mDeviceContext->DrawBitmap(
    texture,
    destRect,
    1.0,
    D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
    sourceRect,
    nullptr);
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
