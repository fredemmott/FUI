// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux stub. Always reports invalid; `TitleBar` handles this by rendering
// no icon. A real impl would hook into freedesktop icon themes via Gio /
// xdg-icon-resource.

#include <FredEmmott/GUI/IconProvider.hpp>

namespace FredEmmott::GUI {
namespace {

class NullIconProvider final : public IconProvider {
 public:
  [[nodiscard]] SoftwareBitmap GetBestSoftwareBitmap(uint16_t) const override {
    return {};
  }
  [[nodiscard]] bool IsValid() const override {
    return false;
  }
};

}// namespace

ApplicationIconProvider::ApplicationIconProvider()
  : mImpl(std::make_unique<NullIconProvider>()) {
}

ApplicationIconProvider::~ApplicationIconProvider() = default;

}// namespace FredEmmott::GUI
