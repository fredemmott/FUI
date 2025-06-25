// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "renderer_detail.hpp"

#include <optional>
#include <stdexcept>

namespace FredEmmott::GUI::renderer_detail {

namespace {
std::optional<RenderAPI>& GetStorage() {
  static std::optional<RenderAPI> storage;
  return storage;
}
}// namespace

RenderAPI GetRenderAPI() {
  if (const auto& ret = GetStorage(); ret.has_value()) {
    return ret.value();
  }
  throw std::logic_error("GetRenderer() called before SetRenderer()");
}

void SetRenderAPI(const RenderAPI value) {
  auto& s = GetStorage();
  if (s.has_value()) {
    if (s.value() != value) [[unlikely]] {
      throw std::logic_error("SetRenderer() called with different results");
    }
    return;
  }
  s = value;
}

}// namespace FredEmmott::GUI::renderer_detail
