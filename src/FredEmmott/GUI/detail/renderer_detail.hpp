// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Font.hpp>
#include <memory>
#include <string_view>

namespace FredEmmott::GUI::renderer_detail {

struct FontMetricsProvider {
  virtual ~FontMetricsProvider() = default;
  virtual float MeasureTextWidth(const Font&, std::string_view) const = 0;
  virtual Font::Metrics GetFontMetrics(const Font&) const = 0;
};

enum class RenderAPI {
  Skia,
  Direct2D,
};

[[nodiscard]]
bool HaveRenderAPI(RenderAPI required);
/// Throws an std::logic_error if the renderer is already set
void SetRenderAPI(
  RenderAPI,
  std::string_view details,
  std::unique_ptr<FontMetricsProvider>);
/// Throws an std::logic_error if the renderer has not yet been set
RenderAPI GetRuntimeRenderAPI();
constexpr RenderAPI GetRenderAPI() {
  if constexpr (Config::HaveSingleBackend) {
    if constexpr (Config::HaveDirect2D) {
      return RenderAPI::Direct2D;
    }
    if constexpr (Config::HaveSkia) {
      return RenderAPI::Skia;
    }
  } else {
    return GetRuntimeRenderAPI();
  }
}

FontMetricsProvider* GetFontMetricsProvider();
/// Human-readable detailed name
std::string_view GetRenderAPIDetails();

}// namespace FredEmmott::GUI::renderer_detail