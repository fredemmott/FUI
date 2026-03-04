// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Direct2DRenderer.hpp"

#include <d3d11.h>

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Font.hpp>
#include <FredEmmott/GUI/Rect.hpp>
#include <FredEmmott/GUI/SoftwareBitmap.hpp>
#include <felly/overload.hpp>
#include <felly/scope_exit.hpp>
#include <numbers>

#include "assert.hpp"
#include "detail/direct_write_detail/DirectWriteFontProvider.hpp"
#include "detail/win32_detail.hpp"

using namespace FredEmmott::GUI::direct_write_detail;
using namespace FredEmmott::GUI::win32_detail;
using namespace FredEmmott::GUI::font_detail;

namespace FredEmmott::GUI {

namespace {

template <std::floating_point T>
constexpr T DegreesToRadians(const T degrees) {
  return degrees * std::numbers::pi_v<float> / static_cast<T>(180);
}

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
  const DWORD dpi,
  const DeviceResources& resources,
  std::shared_ptr<GPUCompletionFlag> frameCompletionFlag)
  : mDPI(dpi),
    mDPIScale(dpi / static_cast<FLOAT>(USER_DEFAULT_SCREEN_DPI)),
    mDeviceResources(resources),
    mFrameCompletionFlag(std::move(frameCompletionFlag)) {}

Direct2DRenderer::~Direct2DRenderer() {
  FUI_ASSERT(mStateStack.empty());
}

void Direct2DRenderer::PushLayer(const float alpha) {
  mStateStack.emplace();
  auto& state = mStateStack.top();
  const auto ctx = mDeviceResources.mD2DDeviceContext;
  ctx->GetTransform(&state.mTransform);

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

  ctx->PushLayer(&params, nullptr);
  state.mHaveNativeLayer = true;
}

void Direct2DRenderer::PopLayer() {
  const auto pop = felly::scope_exit([this] { mStateStack.pop(); });

  const auto ctx = mDeviceResources.mD2DDeviceContext;

  auto& state = mStateStack.top();
  if (state.mHaveNativeLayer) {
    ctx->PopLayer();
  }
  ctx->SetTransform(state.mTransform);
}

void Direct2DRenderer::PushClipRect(const Rect& rect) {
  mDeviceResources.mD2DDeviceContext->PushAxisAlignedClip(
    rect, D2D1_ANTIALIAS_MODE_ALIASED);
}

void Direct2DRenderer::PopClipRect() {
  mDeviceResources.mD2DDeviceContext->PopAxisAlignedClip();
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
  mDeviceResources.mD2DDeviceContext->FillRectangle(
    rect, brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get());
}

void Direct2DRenderer::StrokeRect(
  const Brush& brush,
  const Rect& rect,
  const float thickness) {
  mDeviceResources.mD2DDeviceContext->DrawRectangle(
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
  mDeviceResources.mD2DDeviceContext->DrawLine(
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
  mDeviceResources.mD2DDeviceContext->FillRoundedRectangle(
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
  mDeviceResources.mD2DDeviceContext->GetFactory(&factory);
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
  mDeviceResources.mD2DDeviceContext->FillGeometry(
    path.get(), brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get(), nullptr);
}

void Direct2DRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius,
  float thickness) {
  mDeviceResources.mD2DDeviceContext->DrawRoundedRectangle(
    {rect, radius, radius},
    brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get(),
    thickness == 0 ? 1 : thickness,
    nullptr);
}

void Direct2DRenderer::StrokeArc(
  const Brush& brush,
  const Rect& rect,
  const float startAngle,
  const float sweepAngle,
  const float thickness,
  const StrokeCap strokeCap) {
  if (
    strokeCap == StrokeCap::None
    && std::abs(sweepAngle) < std::numeric_limits<float>::epsilon()) {
    return;
  }

  const auto center = rect.GetCenter();
  const auto radiusX = rect.GetWidth() / 2;
  const auto radiusY = rect.GetHeight() / 2;
  const auto endAngle = startAngle + sweepAngle;

  const D2D1_POINT_2F startPoint {
    center.mX + radiusX * std::cos(DegreesToRadians(startAngle)),
    center.mY + radiusY * std::sin(DegreesToRadians(startAngle)),
  };
  const D2D1_POINT_2F endPoint {
    center.mX + radiusX * std::cos(DegreesToRadians(endAngle)),
    center.mY + radiusY * std::sin(DegreesToRadians(endAngle)),
  };

  wil::com_ptr<ID2D1PathGeometry> path;
  CheckHResult(mDeviceResources.mD2DFactory->CreatePathGeometry(&path));
  wil::com_ptr<ID2D1GeometrySink> sink;
  CheckHResult(path->Open(&sink));

  sink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN_HOLLOW);
  const D2D1_ARC_SEGMENT arc {
    .point = endPoint,
    .size = {radiusX, radiusY},
    .rotationAngle = 0.0f,
    .sweepDirection = sweepAngle > 0 ? D2D1_SWEEP_DIRECTION_CLOCKWISE
                                     : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
    .arcSize
    = std::abs(sweepAngle) > 180 ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL,
  };
  sink->AddArc(arc);
  sink->EndFigure(D2D1_FIGURE_END_OPEN);
  CheckHResult(sink->Close());

  mDeviceResources.mD2DDeviceContext->DrawGeometry(
    path.get(),
    brush.as<wil::com_ptr<ID2D1Brush>>(this, rect).get(),
    thickness,
    GetStrokeStyle(strokeCap));
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
  mDeviceResources.mD2DDeviceContext->DrawText(
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
  const ImportedTexture::HandleKind kind,
  HANDLE const handle) const {
  using enum ImportedTexture::HandleKind;

  wil::com_ptr<ID3D11Texture2D> texture;
  const auto d3d = mDeviceResources.mD3DDevice;
  switch (kind) {
    case LegacySharedHandle:
      CheckHResult(
        d3d->OpenSharedResource(handle, IID_PPV_ARGS(texture.put())));
      break;
    case NTHandle:
      CheckHResult(
        d3d->OpenSharedResource1(handle, IID_PPV_ARGS(texture.put())));
      break;
  }
  const auto surface = texture.query<IDXGISurface>();
  wil::com_ptr<ID2D1Bitmap1> bitmap;
  CheckHResult(mDeviceResources.mD2DDeviceContext->CreateBitmapFromDxgiSurface(
    surface.get(), nullptr, bitmap.put()));
  auto ret = std::make_unique<ImportedDirect2DTexture>();
  ret->mBitmap = std::move(bitmap);
  return ret;
}

std::unique_ptr<ImportedTexture> Direct2DRenderer::ImportSoftwareBitmap(
  const SoftwareBitmap& in) const {
  using PL = SoftwareBitmap::PixelLayout;
  using AF = SoftwareBitmap::AlphaFormat;
  FUI_ASSERT(in.mPixelLayout == PL::BGRA32);
  FUI_ASSERT(in.mAlphaFormat == AF::Premultiplied);

  float dpiX {};
  float dpiY {};
  mDeviceResources.mD2DDeviceContext->GetDpi(&dpiX, &dpiY);

  const auto props = D2D1::BitmapProperties1(
    D2D1_BITMAP_OPTIONS_NONE,
    D2D1::PixelFormat(
      DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
    dpiX,
    dpiY);
  const auto pitch = static_cast<uint32_t>(in.mWidth) * 4;// 32 bpp = 4 bytes

  wil::com_ptr<ID2D1Bitmap1> out;
  CheckHResult(mDeviceResources.mD2DDeviceContext->CreateBitmap(
    D2D1_SIZE_U {in.mWidth, in.mHeight},
    in.mData.data(),
    pitch,
    &props,
    out.put()));
  auto ret = std::make_unique<ImportedDirect2DTexture>();
  ret->mBitmap = std::move(out);
  return ret;
}

std::unique_ptr<ImportedFence> Direct2DRenderer::ImportFence(
  HANDLE const handle) const {
  wil::com_ptr<ID3D11Fence> fence;
  CheckHResult(mDeviceResources.mD3DDevice->OpenSharedFence(
    handle, IID_PPV_ARGS(fence.put())));
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
  const auto texture
    = IMPL_CAST<ImportedDirect2DTexture*>(rawTexture)->mBitmap.get();

  FUI_ASSERT(texture);
  if (rawFence) {
    FUI_ASSERT(fenceValue > 0, "A wait for fence 0 always succeeds");
    const auto fence
      = IMPL_CAST<ImportedDirect3DFence*>(rawFence)->mFence.get();
    FUI_ASSERT(fence);
    CheckHResult(mDeviceResources.mD3DDeviceContext->Wait(fence, fenceValue));
  }
  mDeviceResources.mD2DDeviceContext->DrawBitmap(
    texture,
    destRect,
    1.0,
    D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
    sourceRect,
    nullptr);
}

std::shared_ptr<GPUCompletionFlag>
Direct2DRenderer::GetGPUCompletionFlagForCurrentFrame() const {
  return mFrameCompletionFlag;
}

void Direct2DRenderer::PostTransform(const D2D1_MATRIX_3X2_F& transform) {
  const auto ctx = mDeviceResources.mD2DDeviceContext;
  D2D1_MATRIX_3X2_F combined {};
  ctx->GetTransform(&combined);
  combined = transform * combined;
  ctx->SetTransform(combined);
}
ID2D1StrokeStyle* Direct2DRenderer::GetStrokeStyle(const StrokeCap cap) const {
  switch (cap) {
    case StrokeCap::None:
      return nullptr;
    case StrokeCap::Round:
      return mDeviceResources.mD2DStrokeStyleRoundCap;
    case StrokeCap::Square:
      return mDeviceResources.mD2DStrokeStyleSquareCap;
  }
  FUI_FATAL("Invalid cap style: {}", std::to_underlying(cap));
}

void Direct2DRenderer::Clear(const Color& color) {
  mDeviceResources.mD2DDeviceContext->Clear(color.as<D2D1_COLOR_F>());
}

}// namespace FredEmmott::GUI
