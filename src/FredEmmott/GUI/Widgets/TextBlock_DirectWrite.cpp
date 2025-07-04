// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <Windows.h>

#include <print>

#include "FredEmmott/GUI/detail/direct_write_detail/DirectWriteFontProvider.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "TextBlock.hpp"

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::font_detail;
using namespace FredEmmott::GUI::win32_detail;
using namespace FredEmmott::GUI::direct_write_detail;

namespace FredEmmott::GUI::Widgets {
YGSize TextBlock::MeasureWithDirectWrite(
  float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  float height,
  [[maybe_unused]] YGMeasureMode heightMode) {
  const auto layout = mDirectWriteTextLayout.get();

  if (std::isnan(width)) {
    width = std::numeric_limits<float>::infinity();
  }
  CheckHResult(layout->SetMaxWidth(width));

  if (std::isnan(height)) {
    height = std::numeric_limits<float>::infinity();
  }
  CheckHResult(layout->SetMaxHeight(height));

  DWRITE_TEXT_METRICS metrics {};
  CheckHResult(layout->GetMetrics(&metrics));
  return {
    std::ceil(metrics.width),
    std::ceil(metrics.height),
  };
}

void TextBlock::UpdateDirectWriteTextLayout() {
  mDirectWriteTextLayout.reset();
  const auto dwrite = DirectWriteFontProvider::Get()->mDWriteFactory.get();
  const auto wideText = Utf8ToWide(mText);
  CheckHResult(dwrite->CreateTextLayout(
    wideText.data(),
    wideText.size(),
    mFont.as<DirectWriteFont>().mTextFormat.get(),
    std::numeric_limits<FLOAT>::infinity(),
    std::numeric_limits<FLOAT>::infinity(),
    mDirectWriteTextLayout.put()));
}

void TextBlock::PaintOwnContent(
  ID2D1RenderTarget* rt,
  const Rect& rect,
  const Style& style) const {
  CheckHResult(mDirectWriteTextLayout->SetMaxWidth(rect.GetWidth()));
  rt->DrawTextLayout(
    {rect.GetLeft(), rect.GetTop()},
    mDirectWriteTextLayout.get(),
    style.mColor->GetDirect2DBrush(rt, rect).get(),
    D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}

}// namespace FredEmmott::GUI::Widgets