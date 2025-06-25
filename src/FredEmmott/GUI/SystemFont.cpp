// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemFont.hpp"

#include <FredEmmott/GUI/Font.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/renderer_detail.hpp>
#include <FredEmmott/GUI/detail/system_font_detail.hpp>
#include <stdexcept>

namespace FredEmmott::GUI::SystemFont {

using namespace renderer_detail;

Font Resolve(const Usage usage) {
  switch (GetRenderAPI()) {
#ifdef FUI_ENABLE_SKIA
    case RenderAPI::Skia:
      return ResolveSkiaFont(usage);
#endif
#ifdef FUI_ENABLE_DIRECT2D
    case RenderAPI::Direct2D:
      return ResolveDirectWriteFont(usage);
#endif
    default:
      throw std::logic_error("An unknown render API is selected");
  }
}

Font ResolveGlyphFont(const Usage usage) {
  switch (GetRenderAPI()) {
#ifdef FUI_ENABLE_SKIA
    case RenderAPI::Skia:
      return ResolveGlyphSkiaFont(usage);
#endif
#ifdef FUI_ENABLE_DIRECT2D
    case RenderAPI::Direct2D:
      return ResolveGlyphDirectWriteFont(usage);
#endif
    default:
      throw std::logic_error("An unknown render API is selected");
  }
}

}// namespace FredEmmott::GUI::SystemFont