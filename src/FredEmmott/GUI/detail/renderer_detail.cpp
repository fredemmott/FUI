// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "renderer_detail.hpp"

#include <optional>
#include <stdexcept>

namespace FredEmmott::GUI::renderer_detail {

namespace {

struct Storage {
  RenderAPI mRenderAPI;
  std::string mDetails;
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

RenderAPI GetRuntimeRenderAPI() {
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
  const std::string_view details,
  std::unique_ptr<FontMetricsProvider> fontMetrics) {
  auto& s = GetStorage();
  if (s.has_value()) [[unlikely]] {
    throw std::logic_error("SetRenderAPI() called multiple times");
  }
  s.emplace(value, std::string {details}, std::move(fontMetrics));
}

std::string_view GetRenderAPIDetails() {
  if (const auto& ret = GetStorage(); ret.has_value()) [[likely]] {
    return ret->mDetails;
  }
  throw std::logic_error("GetRenderAPIDetails() called before SetRenderAPI()");
}

namespace {
// Process-wide; default-initialized to "FUI" on first access. Reading
// happens during VkInstance creation in the Skia/Vulkan window backend;
// writes (consumer overrides) happen at startup. Untyped contention isn't
// a concern in practice (single set, then read at window construction).
std::string& MutableApplicationName() {
  static std::string name = "FUI";
  return name;
}
}// namespace

void SetApplicationName(const std::string_view name) {
  MutableApplicationName().assign(name);
}

std::string_view GetApplicationName() noexcept {
  return MutableApplicationName();
}

}// namespace FredEmmott::GUI::renderer_detail
