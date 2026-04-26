// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//

#include "../IconProvider.hpp"

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
