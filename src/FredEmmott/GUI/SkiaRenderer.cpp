// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SkiaRenderer.hpp"

#include <skia/core/SkImage.h>
#include <skia/core/SkRRect.h>

#include <FredEmmott/GUI/detail/renderer_detail.hpp>

#include "assert.hpp"

#ifdef _WIN32
#include <d3d12.h>
#include <skia/gpu/ganesh/GrBackendSemaphore.h>
#include <skia/gpu/ganesh/GrBackendSurface.h>
#include <skia/gpu/ganesh/SkImageGanesh.h>
#include <skia/gpu/ganesh/d3d/GrD3DTypes.h>
#include <wil/com.h>

#include "detail/win32_detail.hpp"
#endif

namespace FredEmmott::GUI {

namespace {
struct ImportedSkiaTexture : ImportedTexture {
  ~ImportedSkiaTexture() override = default;

  sk_sp<SkImage> mSkiaImage;
};

struct ImportedSkiaFence : ImportedFence {
  ~ImportedSkiaFence() override = default;

#ifdef _WIN32
  GrD3DFenceInfo mSkiaFence {};
#endif
};
}// namespace

SkiaRenderer::SkiaRenderer(
  const NativeDevice& nativeDevice,
  SkCanvas* canvas,
  std::shared_ptr<GPUCompletionFlag> frameCompletionFlag)
  : mNativeDevice(nativeDevice),
    mCanvas(canvas),
    mFrameCompletionFlag(std::move(frameCompletionFlag)) {
  FUI_ASSERT(canvas != nullptr);
  FUI_ASSERT(
    renderer_detail::GetRenderAPI() == renderer_detail::RenderAPI::Skia);
}

SkiaRenderer::~SkiaRenderer() {
#ifndef NDEBUG
  FUI_ASSERT(mStackDepth == 0);
#endif
}

void SkiaRenderer::PushLayer(const float alpha) {
  if (std::abs(1.0f - alpha) < std::numeric_limits<float>::epsilon()) {
    mCanvas->save();
  } else {
    mCanvas->saveLayerAlpha(nullptr, alpha);
  }
#ifndef NDEBUG
  ++mStackDepth;
#endif
}

void SkiaRenderer::PopLayer() {
  mCanvas->restore();
#ifndef NDEBUG
  --mStackDepth;
#endif
}

void SkiaRenderer::Clear(const Color& color) {
  mCanvas->clear(color.as<SkColor>());
}

void SkiaRenderer::PushClipRect(const Rect& rect) {
  mCanvas->save();
  mCanvas->clipRect(rect);
#ifndef NDEBUG
  ++mStackDepth;
#endif
}

void SkiaRenderer::PopClipRect() {
  mCanvas->restore();
#ifndef NDEBUG
  --mStackDepth;
#endif
}

void SkiaRenderer::DrawLine(
  const Brush& brush,
  const Point& start,
  const Point& end,
  float thickness) {
  auto paint = brush.as<SkPaint>(this, Rect {start, end});
  paint.setStrokeWidth(thickness);
  mCanvas->drawLine(start.as<SkPoint>(), end.as<SkPoint>(), paint);
}

void SkiaRenderer::Scale(const float x, const float y) {
  mCanvas->scale(x, y);
}

void SkiaRenderer::Translate(const Point& point) {
  mCanvas->translate(point.mX, point.mY);
}

void SkiaRenderer::FillRect(const Brush& brush, const Rect& rect) {
  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mCanvas->drawRect(rect, paint);
}

void SkiaRenderer::StrokeRect(
  const Brush& brush,
  const Rect& rect,
  float thickness) {
  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::Style::kStroke_Style);
  paint.setStrokeWidth(thickness);
  mCanvas->drawRect(rect, paint);
}

void SkiaRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius) {
  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  paint.setAntiAlias(true);
  mCanvas->drawRoundRect(rect, radius, radius, paint);
}

void SkiaRenderer::FillRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float topLeftRadius,
  float topRightRadius,
  float bottomRightRadius,
  float bottomLeftRadius) {
  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  paint.setAntiAlias(true);
  SkVector radii[4] {
    {topLeftRadius, topLeftRadius},
    {topRightRadius, topRightRadius},
    {bottomRightRadius, bottomRightRadius},
    {bottomLeftRadius, bottomLeftRadius},
  };
  SkRRect roundedRect;
  roundedRect.setRectRadii(rect, radii);
  mCanvas->drawRRect(roundedRect, paint);
}

void SkiaRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  float radius,
  float thickness) {
  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(thickness);
  paint.setAntiAlias(true);
  mCanvas->drawRoundRect(rect, radius, radius, paint);
}

void SkiaRenderer::DrawText(
  const Brush& brush,
  const Rect& brushRect,
  const Font& font,
  const std::string_view text,
  const Point& baseline) {
  auto paint = brush.as<SkPaint>(this, brushRect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mCanvas->drawString(
    SkString {text}, baseline.mX, baseline.mY, font.as<SkFont>(), paint);
}

std::unique_ptr<ImportedTexture> SkiaRenderer::ImportTexture(
  [[maybe_unused]] const ImportedTexture::HandleKind kind,
  HANDLE const handle) const {
  FUI_ASSERT(
    kind == ImportedTexture::HandleKind::NTHandle,
    "Only NT HANDLEs are supported when using D3D12 (via Skia)");
  FUI_ASSERT(handle);

  wil::com_ptr<ID3D12Resource> texture;
  auto ret = std::make_unique<ImportedSkiaTexture>();
  win32_detail::CheckHResult(mNativeDevice.mD3DDevice->OpenSharedHandle(
    handle, IID_PPV_ARGS(texture.put())));

  const auto d3dDesc = texture->GetDesc();

  auto colorType = SkColorType::kUnknown_SkColorType;
  switch (d3dDesc.Format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
      colorType = SkColorType::kRGBA_8888_SkColorType;
      break;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
      colorType = SkColorType::kRGBA_8888_SkColorType;
      break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      colorType = SkColorType::kBGRA_8888_SkColorType;
      break;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      colorType = SkColorType::kBGRA_8888_SkColorType;
      break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      colorType = SkColorType::kRGBA_F16_SkColorType;
      break;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
      colorType = SkColorType::kRGBA_1010102_SkColorType;
      break;
    default:
      throw std::runtime_error(
        std::format(
          "Unsupported DXGI format: {}", std::to_underlying(d3dDesc.Format)));
  }

  const GrD3DTextureResourceInfo skiaDesc {
    // AdoptTextureFrom() takes ownership of the texture
    texture.detach(),
    /* alloc = */ nullptr,
    D3D12_RESOURCE_STATE_COMMON,
    d3dDesc.Format,
    d3dDesc.SampleDesc.Count,
    d3dDesc.MipLevels,
    d3dDesc.SampleDesc.Quality,
  };

  const GrBackendTexture skiaTexture(d3dDesc.Width, d3dDesc.Height, skiaDesc);
  ret->mSkiaImage = SkImages::AdoptTextureFrom(
    mNativeDevice.mSkiaContext,
    skiaTexture,
    kTopLeft_GrSurfaceOrigin,
    colorType);

  return std::move(ret);
}

std::unique_ptr<ImportedFence> SkiaRenderer::ImportFence(
  const HANDLE handle) const {
  FUI_ASSERT(handle);
  auto ret = std::make_unique<ImportedSkiaFence>();
  win32_detail::CheckHResult(mNativeDevice.mD3DDevice->OpenSharedHandle(
    handle, IID_PPV_ARGS(&ret->mSkiaFence.fFence)));
  return std::move(ret);
}

void SkiaRenderer::DrawTexture(
  const Rect& sourceRect,
  const Rect& destRect,
  ImportedTexture* const rawTexture,
  ImportedFence* const rawFence,
  const uint64_t fenceValue) {
  FUI_ASSERT(rawTexture);
  FUI_ASSERT(rawFence);
  FUI_ASSERT(fenceValue > 0, "A wait for fence 0 always succeeds");

#ifndef NDEBUG
#define IMPL_CAST dynamic_cast
#else
#define IMPL_CAST static_cast
#endif
  const auto skiaImage
    = IMPL_CAST<ImportedSkiaTexture*>(rawTexture)->mSkiaImage.get();
  const auto fence = &IMPL_CAST<ImportedSkiaFence*>(rawFence)->mSkiaFence;
#undef IMPL_CAST
  FUI_ASSERT(skiaImage && fence);

#ifdef _WIN32
  fence->fValue = fenceValue;
  GrBackendSemaphore semaphore;
  semaphore.initDirect3D(*fence);
  mNativeDevice.mSkiaContext->wait(1, &semaphore, false);
#endif

  mCanvas->drawImageRect(
    skiaImage,
    sourceRect,
    destRect,
    SkSamplingOptions {SkFilterMode::kNearest},
    nullptr,
    SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint);
}

std::shared_ptr<GPUCompletionFlag>
SkiaRenderer::GetGPUCompletionFlagForCurrentFrame() const {
  return mFrameCompletionFlag;
}

}// namespace FredEmmott::GUI