// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <wil/com.h>

#include <FredEmmott/utility/bitflag_enums.hpp>

#include "Brush.hpp"
#include "Color.hpp"
#include "CornerRadius.hpp"
#include "Font.hpp"
#include "FredEmmott/utility/almost_equal.hpp"
#include "Point.hpp"
#include "Rect.hpp"

namespace FredEmmott::GUI {
struct CornerRadius;

struct SoftwareBitmap;

/** VRAM imported from some other API
 *
 * Only 2D textures are supported.
 */
struct ImportedTexture {
#ifdef _WIN32
  enum class HandleKind {
    LegacySharedHandle,// D3D11_RESOURCE_MISC_SHARED
    NTHandle,// D3D11_RESOURCE_MISC_SHARED_NTHANDLE
  };
#endif

  virtual ~ImportedTexture() = default;
};

/** A CPU <-> GPU synchronization primitive with increasing values.
 *
 * For example, a Direct3D fence, Vulkan timeline semaphore, or similar.
 *
 * **NOT** like a Vulkan fence - those are just bool flags, not incrementing
 * values.
 */
struct ImportedFence {
  virtual ~ImportedFence() = default;
};

struct GPUCompletionFlag {
  virtual ~GPUCompletionFlag() = default;
  [[nodiscard]]
  virtual bool IsComplete() const = 0;
  virtual void Wait() const = 0;
};

enum class StrokeCap {
  /** End exactly at the end of the stroke, no cap.
   *
   * - Direct2D: D2D1_CAP_STYLE_FLAT
   * - Skia: kButt_Cap
   */
  None,
  /** End at (thickness / 2) later, but square ends.
   *
   * - Direct2D: D2D1_CAP_STYLE_SQUARE
   * - Skia: kSquare_Cap
   */
  Square,
  /** A half-circle extending past the end, with diameter == thickness
   *
   * - Direct2D: D2D1_CAP_STYLE_ROUND
   * - Skia: kRound_Cap
   */
  Round,
};

enum class Edges : uint8_t {
  None,
  Top = 1,
  Bottom = 2,
  Left = 4,
  Right = 8,
  All = Top | Bottom | Left | Right,
};
consteval bool is_bitflag_enum(std::type_identity<Edges>) {
  return true;
};

class Renderer {
 public:
  virtual ~Renderer() = default;

  /// Push the clipping region and transform
  virtual void PushLayer(float alpha = 1.0f) = 0;
  virtual void PopLayer() = 0;
  auto ScopedLayer(float alpha = 1.0f) {
    PushLayer(alpha);
    return wil::scope_exit([this] { PopLayer(); });
  }

  virtual void Clear(const Color&) = 0;

  virtual void PushClipRect(const Rect&) = 0;
  virtual void PopClipRect() = 0;
  auto ScopedClipRect(const Rect& rect) {
    PushClipRect(rect);
    return wil::scope_exit([this] { PopClipRect(); });
  }

  [[nodiscard]]
  virtual uint64_t GetPhysicalLength(uint64_t dipLength) = 0;
  [[nodiscard]]
  virtual float GetPhysicalLength(float dipLength) = 0;

  virtual void Scale(float x, float y) = 0;

  void Scale(float scale) {
    this->Scale(scale, scale);
  }

  virtual void Translate(const Point&) = 0;
  void Translate(float x, float y) {
    this->Translate({x, y});
  }

  virtual void Rotate(float degrees, const Point& center) = 0;

  virtual void FillRect(const Brush& brush, const Rect& rect) = 0;
  virtual void
  StrokeRect(const Brush& brush, const Rect& rect, float thickness = 0) = 0;
  virtual void DrawLine(
    const Brush& brush,
    const Point& start,
    const Point& end,
    float thickness = 0,
    StrokeCap = StrokeCap::None) = 0;

  virtual void FillRoundedRect(
    const Brush& brush,
    const Rect& rect,
    const CornerRadius&) = 0;
  virtual void StrokeRoundedRect(
    const Brush& brush,
    const Rect& rect,
    const CornerRadius&,
    Edges edges = Edges::All,
    float thickness = 1) = 0;

  /** Stroke an arc.
   *
   * `rect` is the nominal box; the actual arc will bleed outside, due to
   * thickness.
   *
   * - startAngle is in degrees, and starts at 'east' (+x)
   * - sweepAngle is in degrees, and is an offset from `startAngle`
   */
  virtual void StrokeArc(
    const Brush& brush,
    const Rect& rect,
    float startAngle,
    float sweepAngle,
    float thickness = 1,
    StrokeCap strokeCap = StrokeCap::None) = 0;
  virtual void
  StrokeEllipse(const Brush& brush, const Rect& rect, float thickness = 1) = 0;

  void FillEllipse(const Brush& brush, const Rect& rect) {
    FUI_ASSERT(
      utility::almost_equal(rect.GetHeight(), rect.GetWidth()),
      "FillEllipse is not fully implemented, and currently wraps "
      "FillRoundedRect()");
    FillRoundedRect(brush, rect, CornerRadius {rect.GetHeight() / 2});
  }

  virtual void DrawText(
    const Brush& brush,
    const Rect& brushRect,
    const Font& font,
    std::string_view text,
    const Point& baseline) = 0;

  [[nodiscard]]
  virtual std::unique_ptr<ImportedTexture> ImportTexture(
    ImportedTexture::HandleKind,
    HANDLE) const = 0;
  [[nodiscard]]
  virtual std::unique_ptr<ImportedTexture> ImportSoftwareBitmap(
    const SoftwareBitmap& bitmap) const = 0;

  [[nodiscard]]
  virtual std::unique_ptr<ImportedFence> ImportFence(HANDLE) const = 0;

  virtual void DrawTexture(
    const Rect& sourceRect,
    const Rect& destRect,
    ImportedTexture* texture,
    ImportedFence* fence,
    uint64_t fenceValue) = 0;

  virtual std::shared_ptr<GPUCompletionFlag>
  GetGPUCompletionFlagForCurrentFrame() const = 0;
};

}// namespace FredEmmott::GUI