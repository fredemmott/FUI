// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "renderer_detail.hpp"

#include <optional>
#include <stdexcept>

namespace FredEmmott::GUI::renderer_detail {

namespace {

struct Storage {
  RenderAPI mRenderAPI;
  std::unique_ptr<FontMetricsProvider> mFontMetricsProvider;
};

std::optional<Storage>& GetStorage() {
  static std::optional<Storage> storage;
  return storage;
}
}// namespace

bool HaveRenderAPI(RenderAPI required) {
  auto& storage = GetStorage();
  if (!storage.has_value()) {
    return false;
  }
  return storage->mRenderAPI == required;
}

RenderAPI GetRenderAPI() {
  if (const auto& ret = GetStorage(); ret.has_value()) [[likely]] {
    return ret->mRenderAPI;
  }
  throw std::logic_error("GetRenderAPI() called before SetRenderAPI()");
}

FontMetricsProvider* GetFontMetricsProvider() {
  if (const auto& ret = GetStorage(); ret.has_value()) [[likely]] {
    return ret->mFontMetricsProvider.get();
  }
  throw std::logic_error(
    "GetFontMetricsProvider() called before SetRenderAPI()");
}

void SetRenderAPI(
  const RenderAPI value,
  std::unique_ptr<FontMetricsProvider> fontMetrics) {
  auto& s = GetStorage();
  if (!s.has_value()) [[likely]] {
    s.emplace(value, std::move(fontMetrics));
    return;
  }
  throw std::logic_error("SetRenderAPI() called multiple times");
}

}// namespace FredEmmott::GUI::renderer_detail
