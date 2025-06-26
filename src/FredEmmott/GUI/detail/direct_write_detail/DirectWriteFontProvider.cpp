// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "DirectWriteFontProvider.hpp"

#include <FredEmmott/GUI/detail/direct2d_detail.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>

namespace FredEmmott::GUI::direct_write_detail {
using namespace win32_detail;
using namespace direct2d_detail;

float DirectWriteFontProvider::MeasureTextWidth(
  const Font& font,
  const std::string_view text) const {
  using namespace font_detail;
  if (text.empty()) {
    return 0.0f;
  }
  if (!font) [[unlikely]] {
    return std::numeric_limits<float>::quiet_NaN();
  }
  const auto wideText = win32_detail::Utf8ToWide(text);

  const auto props = font.as<DirectWriteFont>();

  wil::com_ptr<IDWriteTextLayout> textLayout;
  const HRESULT hr = mDWriteFactory->CreateTextLayout(
    wideText.c_str(),
    static_cast<UINT32>(wideText.length()),
    props.mTextFormat.get(),
    // Use a large width to ensure text is not wrapped
    FLT_MAX,
    // Height doesn't matter for width measurement
    FLT_MAX,
    textLayout.put());

  if (FAILED(hr)) {
    return 0.0f;
  }

  DWRITE_TEXT_METRICS metrics {};
  if (FAILED(textLayout->GetMetrics(&metrics))) {
    return 0.f;
  }

  return direct2d_detail::DIPsToPixels(metrics.width);
}
Font::Metrics DirectWriteFontProvider::GetFontMetrics(const Font& font) const {
  using namespace font_detail;
  const auto props = font.as<DirectWriteFont>();

  // Get the font collection from the text format
  wil::com_ptr<IDWriteFontCollection> fontCollection;
  CheckHResult(props.mTextFormat->GetFontCollection(fontCollection.put()));

  // Find the font family
  UINT32 familyIndex;
  BOOL exists;
  win32_detail::CheckHResult(
    fontCollection->FindFamilyName(props.mName.c_str(), &familyIndex, &exists));
  if (!exists) {
    return {};
  }

  // Get the font family
  wil::com_ptr<IDWriteFontFamily> fontFamily;
  CheckHResult(fontCollection->GetFontFamily(familyIndex, fontFamily.put()));

  // Get a font from the family that matches weight, style, etc.
  wil::com_ptr<IDWriteFont> dwriteFont;
  DWRITE_FONT_WEIGHT weight = props.mTextFormat->GetFontWeight();
  DWRITE_FONT_STYLE style = props.mTextFormat->GetFontStyle();
  DWRITE_FONT_STRETCH stretch = props.mTextFormat->GetFontStretch();

  CheckHResult(
    fontFamily->GetFirstMatchingFont(weight, stretch, style, dwriteFont.put()));

  // Get font metrics
  DWRITE_FONT_METRICS fontMetrics;
  dwriteFont->GetMetrics(&fontMetrics);

  // Convert design units to DIPs and then to pixels
  const float fontSize = props.mTextFormat->GetFontSize();
  const float designUnitsToPixels
    = (fontSize / fontMetrics.designUnitsPerEm) * (72.f / 96);

  // Calculate font metrics in pixels
  const float ascent = fontMetrics.ascent * designUnitsToPixels;
  const float descent = fontMetrics.descent * designUnitsToPixels;
  const float lineGap = fontMetrics.lineGap * designUnitsToPixels;

  // Calculate line height
  const float lineHeight = ascent + descent + lineGap;

  return Font::Metrics {
    .mSize = props.mSize,
    .mLineSpacing = ascent + descent + lineGap,
    .mDescent = descent,
  };
}
}// namespace FredEmmott::GUI::direct_write_detail