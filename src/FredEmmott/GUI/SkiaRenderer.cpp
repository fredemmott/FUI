// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SkiaRenderer.hpp"

#include <skia/core/SkImage.h>
#include <skia/core/SkRRect.h>

#include <FredEmmott/GUI/detail/renderer_detail.hpp>

#include "SoftwareBitmap.hpp"
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

SkPaint::Cap GetSkiaStrokeCap(const StrokeCap cap) {
  using enum StrokeCap;
  switch (cap) {
    case None:
      return SkPaint::kButt_Cap;
    case Round:
      return SkPaint::kRound_Cap;
    case Square:
      return SkPaint::kSquare_Cap;
  }
  std::unreachable();
}
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
#ifdef FUI_DEBUG
  FUI_ASSERT(mStackDepth == 0);
#endif
}

void SkiaRenderer::PushLayer(const float alpha) {
  if (std::abs(1.0f - alpha) < std::numeric_limits<float>::epsilon()) {
    mCanvas->save();
  } else {
    mCanvas->saveLayerAlphaf(nullptr, alpha);
  }
#ifdef FUI_DEBUG
  ++mStackDepth;
#endif
}

void SkiaRenderer::PopLayer() {
  mCanvas->restore();
#ifdef FUI_DEBUG
  --mStackDepth;
#endif
}

void SkiaRenderer::Clear(const Color& color) {
  mCanvas->clear(color.as<SkColor>());
}

void SkiaRenderer::PushClipRect(const Rect& rect) {
  mCanvas->save();
  mCanvas->clipRect(rect);
#ifdef FUI_DEBUG
  ++mStackDepth;
#endif
}

void SkiaRenderer::PopClipRect() {
  mCanvas->restore();
#ifdef FUI_DEBUG
  --mStackDepth;
#endif
}

void SkiaRenderer::DrawLine(
  const Brush& brush,
  const Point& start,
  const Point& end,
  const float thickness,
  const StrokeCap strokeCap) {
  auto paint = brush.as<SkPaint>(this, Rect {start, end});
  paint.setStrokeWidth(thickness);
  paint.setAntiAlias(true);
  paint.setStrokeCap(GetSkiaStrokeCap(strokeCap));

  mCanvas->drawLine(start.as<SkPoint>(), end.as<SkPoint>(), paint);
}

void SkiaRenderer::Scale(const float x, const float y) {
  mCanvas->scale(x, y);
}

void SkiaRenderer::Translate(const Point& point) {
  mCanvas->translate(point.mX, point.mY);
}

void SkiaRenderer::Rotate(const float degrees, const Point& center) {
  mCanvas->rotate(degrees, center.mX, center.mY);
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
  const CornerRadius& radii) {
  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  paint.setAntiAlias(true);

  if (radii.IsUniform()) {
    const auto radius = radii.GetUniformValue();
    mCanvas->drawRoundRect(rect, radius, radius, paint);
    return;
  }

  const auto tl = radii.GetTopLeft();
  const auto tr = radii.GetTopRight();
  const auto br = radii.GetBottomRight();
  const auto bl = radii.GetBottomLeft();

  const SkVector skRadii[4] {
    {tl, tl},
    {tr, tr},
    {br, br},
    {bl, bl},
  };
  SkRRect roundedRect;
  roundedRect.setRectRadii(rect, skRadii);
  mCanvas->drawRRect(roundedRect, paint);
}

void SkiaRenderer::StrokeRoundedRect(
  const Brush& brush,
  const Rect& rect,
  const CornerRadius& radii,
  const EdgeFlags edges,
  const float thickness) {
  static constexpr auto Epsilon = std::numeric_limits<float>::epsilon();

  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(thickness);
  paint.setAntiAlias(true);
  const auto tl = radii.GetTopLeft();
  const auto tr = radii.GetTopRight();
  const auto br = radii.GetBottomRight();
  const auto bl = radii.GetBottomLeft();

  if (edges == EdgeFlags::All) {
    const SkVector skRadii[4] {
      {tl, tl},
      {tr, tr},
      {br, br},
      {bl, bl},
    };
    SkRRect roundedRect;
    roundedRect.setRectRadii(rect, skRadii);
    mCanvas->drawRRect(roundedRect, paint);
    return;
  }

  SkPath path;
  bool attached = false;
  const auto MoveOrLineTo = [&](const Point& point) {
    if (std::exchange(attached, true)) {
      path.lineTo(point.mX, point.mY);
    } else {
      path.moveTo(point.mX, point.mY);
    }
  };
  const auto LineTo
    = [&path](const Point& point) { path.lineTo(point.mX, point.mY); };

  if ((edges & EdgeFlags::Top) == EdgeFlags::Top) {
    MoveOrLineTo(rect.GetTopLeft() + Point {tl, 0});
    LineTo(rect.GetTopRight() - Point {tr, 0});
  } else {
    FUI_ASSERT(tl < Epsilon);
    FUI_ASSERT(tr < Epsilon);
    attached = false;
  }

  if (tr > Epsilon) {
    FUI_ASSERT((edges & EdgeFlags::Top) == EdgeFlags::Top);
    FUI_ASSERT((edges & EdgeFlags::Right) == EdgeFlags::Right);
    FUI_ASSERT(attached);
    // Rect represents the full circle, not the arc
    path.arcTo(
      SkRect::MakeLTRB(
        rect.GetRight() - (2 * tr),
        rect.GetTop(),
        rect.GetRight(),
        rect.GetTop() + (2 * tr)),
      270,// 0 degrees is 3'o clock, start at 12
      90,// Sweep 90 degrees,
      false);
  }

  if ((edges & EdgeFlags::Right) == EdgeFlags::Right) {
    MoveOrLineTo(rect.GetTopRight() + Point {0, tr});
    LineTo(rect.GetBottomRight() - Point {0, br});
  } else {
    FUI_ASSERT(tr < Epsilon);
    FUI_ASSERT(br < Epsilon);
    attached = false;
  }

  if (br > Epsilon) {
    FUI_ASSERT((edges & EdgeFlags::Right) == EdgeFlags::Right);
    FUI_ASSERT((edges & EdgeFlags::Bottom) == EdgeFlags::Bottom);
    FUI_ASSERT(attached);
    path.arcTo(
      SkRect::MakeLTRB(
        rect.GetRight() - (2 * br),
        rect.GetBottom() - (2 * br),
        rect.GetRight(),
        rect.GetBottom()),
      0,
      90,
      false);
  }

  if ((edges & EdgeFlags::Bottom) == EdgeFlags::Bottom) {
    MoveOrLineTo(rect.GetBottomRight() - Point {br, 0});
    LineTo(rect.GetBottomLeft() + Point {bl, 0});
  } else {
    FUI_ASSERT(br < Epsilon);
    FUI_ASSERT(bl < Epsilon);
    attached = false;
  }

  if (bl > Epsilon) {
    FUI_ASSERT((edges & EdgeFlags::Bottom) == EdgeFlags::Bottom);
    FUI_ASSERT((edges & EdgeFlags::Left) == EdgeFlags::Left);
    FUI_ASSERT(attached);
    path.arcTo(
      SkRect::MakeLTRB(
        rect.GetLeft(),
        rect.GetBottom() - (2 * bl),
        rect.GetLeft() + (2 * bl),
        rect.GetBottom()),
      90,// 90 degrees from 3 o'clock -> 6 o'clock
      90,
      false);
  }

  if ((edges & EdgeFlags::Left) == EdgeFlags::Left) {
    MoveOrLineTo(rect.GetBottomLeft() - Point {0, bl});
    LineTo(rect.GetTopLeft() + Point {0, tl});
  } else {
    FUI_ASSERT(bl < Epsilon);
    FUI_ASSERT(tl < Epsilon);
    attached = false;
  }

  if (tl > Epsilon) {
    FUI_ASSERT((edges & EdgeFlags::Left) == EdgeFlags::Left);
    FUI_ASSERT((edges & EdgeFlags::Top) == EdgeFlags::Top);
    FUI_ASSERT(attached);
    path.arcTo(
      SkRect::MakeLTRB(
        rect.GetLeft(),
        rect.GetTop(),
        rect.GetLeft() + (2 * tl),
        rect.GetTop() + (2 * tl)),
      180,
      90,
      false);
  }

  mCanvas->drawPath(path, paint);
}

void SkiaRenderer::StrokeArc(
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

  if (rect.GetWidth() < thickness || rect.GetHeight() < thickness) {
    return;
  }

  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(thickness);
  paint.setAntiAlias(true);
  paint.setStrokeCap(GetSkiaStrokeCap(strokeCap));
  mCanvas->drawArc(rect, startAngle, sweepAngle, false, paint);
}

void SkiaRenderer::StrokeEllipse(
  const Brush& brush,
  const Rect& rect,
  const float thickness) {
  constexpr auto Epsilon = std::numeric_limits<float>::epsilon();
  if (
    rect.GetWidth() < Epsilon || rect.GetHeight() < Epsilon
    || thickness < Epsilon) {
    return;
  }

  auto paint = brush.as<SkPaint>(this, rect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(thickness);
  paint.setAntiAlias(true);
  mCanvas->drawOval(rect, paint);
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
#ifdef FUI_DEBUG
#define IMPL_CAST dynamic_cast
#else
#define IMPL_CAST static_cast
#endif
  const auto skiaImage
    = IMPL_CAST<ImportedSkiaTexture*>(rawTexture)->mSkiaImage.get();

  FUI_ASSERT(skiaImage);
  if (rawFence) {
    FUI_ASSERT(fenceValue > 0, "A wait for fence 0 always succeeds");
    const auto fence = &IMPL_CAST<ImportedSkiaFence*>(rawFence)->mSkiaFence;
    FUI_ASSERT(fence);

#ifdef _WIN32
    fence->fValue = fenceValue;
    GrBackendSemaphore semaphore;
    semaphore.initDirect3D(*fence);
    mNativeDevice.mSkiaContext->wait(1, &semaphore, false);
#endif
  }

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

std::unique_ptr<ImportedTexture> SkiaRenderer::ImportSoftwareBitmap(
  const SoftwareBitmap& in) const {
  using PL = SoftwareBitmap::PixelLayout;
  using AF = SoftwareBitmap::AlphaFormat;
  FUI_ASSERT(in.mPixelLayout == PL::BGRA32);
  FUI_ASSERT(in.mAlphaFormat == AF::Premultiplied);

  const auto info = SkImageInfo::Make(
    in.mWidth, in.mHeight, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
  const auto pitch = in.mWidth * 4;

  // make a copy so that we don't have to worry about the GPU copy completing
  // asynchronously
  auto skiaData = SkData::MakeWithCopy(in.mData.data(), in.mData.size());
  auto ramImage = SkImages::RasterFromData(info, std::move(skiaData), pitch);
  auto gpuImage = SkImages::TextureFromImage(
    mNativeDevice.mSkiaContext, std::move(ramImage));

  auto ret = std::make_unique<ImportedSkiaTexture>();
  ret->mSkiaImage = std::move(gpuImage);
  return std::move(ret);
}

}// namespace FredEmmott::GUI