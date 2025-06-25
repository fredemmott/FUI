// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Font.hpp>
#include <memory>

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
void SetRenderAPI(RenderAPI, std::unique_ptr<FontMetricsProvider>);
/// Throws an std::logic_error if the renderer has not yet been set
RenderAPI GetRenderAPI();
FontMetricsProvider* GetFontMetricsProvider();

}// namespace FredEmmott::GUI::renderer_detail