// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <FredEmmott/GUI/detail/renderer_detail.hpp>

#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"

namespace FredEmmott::GUI::direct_write_detail {
struct DirectWriteFontProvider final : renderer_detail::FontMetricsProvider {
  DirectWriteFontProvider() = delete;
  DirectWriteFontProvider(wil::com_ptr<IDWriteFactory> dWriteFactory)
    : mDWriteFactory(std::move(dWriteFactory)) {
    win32_detail::CheckHResult(mDWriteFactory->GetSystemFontCollection(
      mSystemFontCollection.put(), FALSE));
  }

  ~DirectWriteFontProvider() override = default;

  float MeasureTextWidth(const Font& font, const std::string_view text)
    const override;

  Font::Metrics GetFontMetrics(const Font& font) const override;

  wil::com_ptr<IDWriteFactory> mDWriteFactory;
  wil::com_ptr<IDWriteFontCollection> mSystemFontCollection;

  static DirectWriteFontProvider* Get() {
    using namespace renderer_detail;
    FUI_ASSERT(GetRenderAPI() == RenderAPI::Direct2D);
#ifndef NDEBUG
    return dynamic_cast<DirectWriteFontProvider*>(GetFontMetricsProvider());
#else
    return static_cast<DirectWriteFontProvider*>(GetFontMetricsProvider());
#endif
  }
};
}// namespace FredEmmott::GUI::direct_write_detail