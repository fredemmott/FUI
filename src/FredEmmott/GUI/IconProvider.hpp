// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#ifdef _WIN32
#include <wil/resource.h>
#endif

namespace FredEmmott::GUI {

struct Bitmap {
  enum class PixelLayout {
    BGRA32,
  };
  enum class AlphaFormat {
    Premultiplied,
  };

  std::vector<std::byte> mData;

  PixelLayout mPixelLayout {};
  AlphaFormat mAlphaFormat {};

  uint16_t mWidth {};
  uint16_t mHeight {};
};

class IconProvider {
 public:
  virtual ~IconProvider() = default;

  [[nodiscard]]
  virtual Bitmap GetBestBitmap(uint16_t edgeLength) = 0;
  [[nodiscard]]
  virtual bool IsValid() const = 0;

#ifdef _WIN32
  [[nodiscard]]
  virtual wil::unique_hicon GetBestHICON(uint16_t edgeLength) = 0;
#endif

  operator bool() const noexcept {
    return this->IsValid();
  }
};

class ApplicationIconProvider final : public IconProvider {
 public:
  ApplicationIconProvider();
  ~ApplicationIconProvider() override;

  [[nodiscard]] Bitmap GetBestBitmap(uint16_t idealEdgeLength) override {
    return mImpl->GetBestBitmap(idealEdgeLength);
  }

  [[nodiscard]] bool IsValid() const override {
    return mImpl->IsValid();
  }

#ifdef _WIN32
  [[nodiscard]]
  wil::unique_hicon GetBestHICON(uint16_t edgeLength) override {
    return mImpl->GetBestHICON(edgeLength);
  }
#endif

 private:
  // Aggregate backed by several other implementations, 'first wins'
  std::unique_ptr<IconProvider> mImpl;
};

}// namespace FredEmmott::GUI