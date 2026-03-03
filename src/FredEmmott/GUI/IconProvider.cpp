// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "IconProvider.hpp"

#include <Windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <wil/resource.h>

#include <array>
#include <felly/numeric_cast.hpp>
#include <felly/overload.hpp>
#include <span>
#include <variant>

#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "assert.hpp"

namespace FredEmmott::GUI {

using namespace win32_detail;

namespace {

struct GetFirstIconResource {
  using resource_id_type
    = std::variant<std::monostate, std::wstring, decltype(MAKEINTRESOURCEW(1))>;

  auto operator()() {
    mResource = {};
    EnumResourceNamesW(
      GetModuleHandle(nullptr),
      RT_GROUP_ICON,
      &EnumCallback,
      reinterpret_cast<LONG_PTR>(this));
    return std::move(mResource);
  }

 private:
  resource_id_type mResource {};

  static BOOL CALLBACK EnumCallback(
    HMODULE,
    [[maybe_unused]] const wchar_t* kind,
    wchar_t* name,
    LONG_PTR userData) {
    auto& self = *reinterpret_cast<GetFirstIconResource*>(userData);

    if (IS_INTRESOURCE(name)) {
      self.mResource = name;
    } else {
      self.mResource.emplace<std::wstring>(name);
    }

    return FALSE;
  }
};

wil::unique_hicon LoadIconWithScaleDown(
  HMODULE const module,
  LPCWSTR resource,
  const uint16_t edgeLength) {
  static const wil::unique_hmodule commctl32 {LoadLibraryW(L"Comctl32.dll")};
  static const auto Impl = std::bit_cast<decltype(&::LoadIconWithScaleDown)>(
    GetProcAddress(commctl32.get(), "LoadIconWithScaleDown"));
  FUI_ASSERT(
    Impl,
    "LoadIconWithScaleDown() is unavailable; either add a PRI-based app icon, "
    "or add Common-Controls 6.0 to your manifest dependencies");

  wil::unique_hicon iconHandle;
  CheckHResult(
    Impl(module, resource, edgeLength, edgeLength, iconHandle.put()));
  return iconHandle;
}

constexpr uint8_t Premultiply(const uint8_t value, const uint8_t alpha) {
  /* Trick from Skia
   *
   * Supposedly originally from Jimm Blinn, "Three Wrongs Make A Right", Nov
   * 1995, but I found books easier to find:
   * - Jimm Blinn's Corner: Dirty Pixels
   * - Jimm Blinn's Corner: Notation, Notation, Notation
   *
   * We want (value * alpha) / 255
   * This is equivalent for 8-bit inputs.
   */
  return (((static_cast<uint32_t>(value) * alpha) + 128) * 257) >> 16;
}

Bitmap BitmapFromHICON(HICON iconHandle) {
  FUI_ASSERT(iconHandle);
  ICONINFOEXW iconInfo {sizeof(iconInfo)};
  if (!GetIconInfoExW(iconHandle, &iconInfo)) [[unlikely]] {
    ThrowHResult(HRESULT_FROM_WIN32(GetLastError()));
  }
  const wil::unique_hbitmap bitmapHandle {iconInfo.hbmColor};
  const wil::unique_hbitmap maskHandle {iconInfo.hbmMask};
  FUI_ASSERT(bitmapHandle, "Icon does not have a color bitmap");

  BITMAP bitmapIn {};
  GetObject(bitmapHandle.get(), sizeof(bitmapIn), &bitmapIn);

  FUI_ASSERT(bitmapIn.bmWidth == bitmapIn.bmHeight);
  const auto edgeLength = felly::numeric_cast<uint16_t>(bitmapIn.bmWidth);
  const auto pixelCount = edgeLength * edgeLength;
  const auto byteCount = pixelCount * 4;
  Bitmap ret {
    .mPixelLayout = Bitmap::PixelLayout::BGRA32,
    .mAlphaFormat = Bitmap::AlphaFormat::Premultiplied,
    .mWidth = edgeLength,
    .mHeight = edgeLength,
  };
  ret.mData.resize(byteCount);
  memset(ret.mData.data(), 255, byteCount);

  BITMAPINFO bi {
    BITMAPINFOHEADER {
      .biSize = sizeof(BITMAPINFOHEADER),
      .biWidth = bitmapIn.bmWidth,
      .biHeight = -bitmapIn.bmHeight,
      .biPlanes = 1,
      .biBitCount = 32,
      .biCompression = BI_RGB,
    },
  };

  const wil::unique_hdc dc {CreateCompatibleDC(nullptr)};
  if (
    GetDIBits(
      dc.get(),
      bitmapHandle.get(),
      0,
      bitmapIn.bmHeight,
      ret.mData.data(),
      &bi,
      DIB_RGB_COLORS)
    == 0) [[unlikely]] {
    ThrowHResult(HRESULT_FROM_WIN32(GetLastError()));
  }

  const auto pixelsBegin
    = reinterpret_cast<std::array<uint8_t, 4>*>(ret.mData.data());
  const auto pixelsEnd = pixelsBegin + pixelCount;

  bool hasAlpha = false;
  for (auto it = pixelsBegin; it != pixelsEnd; ++it) {
    // 0123
    // ||||
    // BGRA
    if (it->at(3)) {
      hasAlpha = true;
      break;
    }
  }

  if (hasAlpha) {
    for (auto it = pixelsBegin; it != pixelsEnd; ++it) {
      auto& p = *it;
      const auto a = p[3];

      p[0] = Premultiply(p[0], a);
      p[1] = Premultiply(p[1], a);
      p[2] = Premultiply(p[2], a);
    }
    return ret;
  }

  FUI_ASSERT(maskHandle);
  GetObject(maskHandle.get(), sizeof(bitmapIn), &bitmapIn);
  FUI_ASSERT(bitmapIn.bmBitsPixel == 1);
  bi.bmiHeader = {
    .biSize = sizeof(BITMAPINFOHEADER),
    .biWidth = bitmapIn.bmWidth,
    .biHeight = -bitmapIn.bmHeight,
    .biPlanes = 1,
    .biBitCount = 1,
  };
  std::vector<std::byte> mask;
  mask.resize(bitmapIn.bmWidthBytes * bitmapIn.bmHeight);
  if (
    GetDIBits(
      dc.get(),
      maskHandle.get(),
      0,
      bitmapIn.bmHeight,
      mask.data(),
      &bi,
      DIB_RGB_COLORS)
    == 0) [[unlikely]] {
    ThrowHResult(HRESULT_FROM_WIN32(GetLastError()));
  }
  for (std::size_t i = 0; i < pixelCount; ++i) {
    const auto x = i % bitmapIn.bmWidth;
    const auto y = i / bitmapIn.bmWidth;
    const auto maskByteIdx = (y * bitmapIn.bmWidthBytes) + (x / 8);
    const auto maskByte = std::bit_cast<uint8_t>(mask.at(maskByteIdx));
    const auto maskBitIndex = (7 - (x % 8));
    const auto maskBitMask = 1 << maskBitIndex;
    // 1 means transparent, 0 means opaque
    const auto transparent = (maskByte & maskBitMask) == maskBitMask;

    auto& pixel = *(pixelsBegin + i);
    if (transparent) {
      pixel = {};
    } else {
      pixel[3] = 0xFF;
    }
  }
  return ret;
}

class DefaultIconProvider final : public IconProvider {
 public:
  DefaultIconProvider();
  ~DefaultIconProvider() override = default;
  [[nodiscard]] Bitmap GetBestBitmap(uint16_t edgeLength) override;
  wil::unique_hicon GetBestHICON(uint16_t edgeLength) override;

  [[nodiscard]] bool IsValid() const override {
    return mLibrary && mResourceID;
  }

 private:
  wil::unique_hmodule mLibrary;
  decltype(MAKEINTRESOURCEW(1)) mResourceID {};
};

DefaultIconProvider::DefaultIconProvider() {
  SHSTOCKICONINFO info {sizeof(info)};
  if (FAILED(SHGetStockIconInfo(SIID_APPLICATION, SHGSI_ICONLOCATION, &info))) {
    FUI_ASSERT(false, "failed to get stock icon info");
    return;
  }

  mLibrary.reset(
    LoadLibraryExW(info.szPath, nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE));
  if (!mLibrary) {
    FUI_ASSERT(false, "Failed to load icon provider library");
    return;
  }

  mResourceID = MAKEINTRESOURCEW(std::abs(info.iIcon));
}

Bitmap DefaultIconProvider::GetBestBitmap(const uint16_t edgeLength) {
  return BitmapFromHICON(GetBestHICON(edgeLength).get());
}

wil::unique_hicon DefaultIconProvider::GetBestHICON(const uint16_t edgeLength) {
  FUI_ASSERT(IsValid());
  return LoadIconWithScaleDown(mLibrary.get(), mResourceID, edgeLength);
}

class AppResourceIconProvider final : public IconProvider {
 public:
  AppResourceIconProvider();
  ~AppResourceIconProvider() override = default;

  [[nodiscard]] Bitmap GetBestBitmap(uint16_t edgeLength) override;
  wil::unique_hicon GetBestHICON(uint16_t edgeLength) override;

  [[nodiscard]]
  bool IsValid() const override {
    return mResourceID.index() != 0;
  }

 private:
  GetFirstIconResource::resource_id_type mResourceID {};
};

AppResourceIconProvider::AppResourceIconProvider() {
  mResourceID = GetFirstIconResource {}();
}

Bitmap AppResourceIconProvider::GetBestBitmap(const uint16_t edgeLength) {
  return BitmapFromHICON(GetBestHICON(edgeLength).get());
}

wil::unique_hicon AppResourceIconProvider::GetBestHICON(uint16_t edgeLength) {
  FUI_ASSERT(IsValid());
  const auto resourceID = std::visit(
    felly::overload {
      [](std::monostate) -> const wchar_t* {
        FUI_ALWAYS_ASSERT(
          false, "Should not be calling GetBestHICON() on an invalid resource");
      },
      [](const std::wstring& name) -> const wchar_t* { return name.data(); },
      [](const wchar_t* opaque) -> const wchar_t* { return opaque; },
    },
    mResourceID);
  return LoadIconWithScaleDown(
    GetModuleHandleW(nullptr), resourceID, edgeLength);
}

}// namespace

ApplicationIconProvider::ApplicationIconProvider() {
  mImpl = std::make_unique<AppResourceIconProvider>();
  if (mImpl->IsValid()) {
    return;
  }
  mImpl = std::make_unique<DefaultIconProvider>();
}

ApplicationIconProvider::~ApplicationIconProvider() = default;

}// namespace FredEmmott::GUI