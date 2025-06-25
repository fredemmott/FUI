// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::renderer_detail {

enum class RenderAPI {
  Skia,
  Direct2D,
};

/// Throws an std::logic_error if the renderer is already set to a different
/// value
void SetRenderAPI(RenderAPI);
/// Throws an std::logic_error if the renderer has not yet been set
RenderAPI GetRenderAPI();

}// namespace FredEmmott::GUI::renderer_detail