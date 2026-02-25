// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SwapChainPanel.hpp"

#include <felly/numeric_cast.hpp>

#include "FredEmmott/GUI/Window.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"

#ifdef FUI_ENABLE_DIRECT2D
#include <d3d11.h>

#include "FredEmmott/GUI/Direct2DRenderer.hpp"
#endif

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace win32_detail;
constexpr auto SwapChainLength = 3;
}// namespace

struct SwapChainPanel::Resources {
  virtual ~Resources() = default;
  std::array<HANDLE, SwapChainLength> mTextureHandles;
  std::array<std::unique_ptr<ImportedTexture>, SwapChainLength>
    mImportedTextures;
  std::array<bool, SwapChainLength> mHandlesAreNew {true};
  BasicSize<DWORD> mTextureSize;

  HANDLE mFenceHandle {};
  std::unique_ptr<ImportedFence> mFence;

  struct D3D11;
};

#ifdef FUI_ENABLE_DIRECT2D
struct SwapChainPanel::Resources::D3D11 : SwapChainPanel::Resources {
  ~D3D11() override = default;

  std::array<wil::com_ptr<ID3D11Texture2D1>, SwapChainLength> mTextures;
  wil::com_ptr<ID3D11Fence> mFence;
  HANDLE mFenceHandle {};
};
#endif

SwapChainPanel::SwapChainPanel(const std::size_t id)
  : Widget(id, LiteralStyleClass {"SwapChainPanel"}, {}) {}

SwapChainPanel::~SwapChainPanel() = default;

SwapChainBeginFrameInfo SwapChainPanel::BeginFrame() {
  mReady.wait(false);
  const auto frameNumber = mNextFrameNumber++;
  const auto idx = frameNumber % SwapChainLength;
  return SwapChainBeginFrameInfo {
    .mSequenceNumber = frameNumber,
    .mTextureSize = {
      static_cast<uint16_t>(mResources->mTextureSize.mWidth),
      static_cast<uint16_t>(mResources->mTextureSize.mHeight),
      },
    .mMustFlushCachedHandles = mResources->mHandlesAreNew[idx],
    .mTexture = mResources->mTextureHandles[idx],
  };
}

void SwapChainPanel::EndFrame(
  const SwapChainBeginFrameInfo& begin,
  const SwapChainEndFrameInfo& end) {
  FUI_ASSERT(begin.mSequenceNumber == mNextFrameNumber - 1);
  const auto idx = begin.mSequenceNumber % SwapChainLength;
  mResources->mHandlesAreNew[idx] = false;

  Submission ret {
    .mTexture = mResources->mImportedTextures[idx].get(),
    .mFenceIsNew = end.mFenceIsNew,
    .mFenceHandle = end.mFence,
    .mFenceValue = end.mFenceValue,
  };
  mContent.lock()->emplace(std::move(ret));
  GetOwnerWindow()->InterruptWaitFrame();
}

void SwapChainPanel::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style&) const {
  const_cast<SwapChainPanel*>(this)->Init(renderer, rect.mSize);

  const auto content = mContent.lock();
  if (!(content && content->has_value())) {
    return;
  }

  const auto r = mResources.get();
  if ((*content)->mFenceIsNew || (*content)->mFenceHandle != r->mFenceHandle) {
    r->mFenceHandle = (*content)->mFenceHandle;
    r->mFence = renderer->ImportFence(r->mFenceHandle);
  }
  renderer->DrawTexture(
    rect, rect, (*content)->mTexture, r->mFence.get(), (*content)->mFenceValue);
}

void SwapChainPanel::Init(Renderer* renderer, const Size& size) {
  if (mReady.test()) {
    return;
  }
  const auto initialized = InitD3D11(renderer, size);
  FUI_ALWAYS_ASSERT(initialized);
  FUI_ALWAYS_ASSERT(!mReady.test_and_set());
  mReady.notify_all();
}

#ifdef FUI_ENABLE_DIRECT2D
bool SwapChainPanel::InitD3D11(Renderer* raw, const Size& size) {
  const auto renderer = direct2d_renderer_cast(raw);
  if (!renderer) {
    return false;
  }
  auto d3d = renderer->GetD3DDevice();
  auto r = std::make_unique<Resources::D3D11>();

  r->mTextureSize = {
    felly::numeric_cast<DWORD>(size.mWidth),
    felly::numeric_cast<DWORD>(size.mHeight),
  };
  const D3D11_TEXTURE2D_DESC1 desc {
    .Width = r->mTextureSize.mWidth,
    .Height = r->mTextureSize.mHeight,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc = {1, 0},
    .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
    .MiscFlags
    = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED,
  };
  for (auto i = 0; i < SwapChainLength; ++i) {
    auto& texture = r->mTextures[i];
    CheckHResult(d3d->CreateTexture2D1(&desc, nullptr, texture.put()));
    const auto resource = texture.query<IDXGIResource1>();
    CheckHResult(resource->CreateSharedHandle(
      nullptr,
      DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
      nullptr,
      &r->mTextureHandles[i]));
    r->mImportedTextures[i] = renderer->ImportTexture(
      ImportedTexture::HandleKind::NTHandle, r->mTextureHandles[i]);
  }

  mResources = std::move(r);
  return true;
}
#endif

}// namespace FredEmmott::GUI::Widgets