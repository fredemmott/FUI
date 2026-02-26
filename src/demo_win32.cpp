// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "demo_win32.hpp"

#include <d3d11.h>
#include <d3d11_4.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

namespace {
struct TextureProducer {
  wil::unique_handle mTexture {};
  wil::unique_handle mFence {};
  std::atomic<uint64_t> mFenceValue {};
  bool mEarlySignal {false};

  static constexpr fui::Size Size {128, 128};

  void WaitUntilReady() {
    mReady.wait(false);
  }

  TextureProducer() {
    mThread = std::jthread {std::bind_front(&TextureProducer::Run, this)};
  }

 private:
  std::atomic_flag mReady;
  std::jthread mThread;

  void Run(const std::stop_token& stop) {
    using fui::win32_detail::CheckHResult;
    wil::com_ptr<ID3D11Device> basicDevice;
    wil::com_ptr<ID3D11DeviceContext> basicContext;
    CheckHResult(D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
        | D3D11_CREATE_DEVICE_DEBUG
#endif
      ,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      basicDevice.put(),
      nullptr,
      basicContext.put()));
    const auto device = basicDevice.query<ID3D11Device5>();
    const auto context = basicContext.query<ID3D11DeviceContext4>();

    wil::com_ptr<ID3D11Fence> fence;
    CheckHResult(device->CreateFence(
      0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(fence.put())));
    CheckHResult(
      fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, mFence.put()));

    wil::com_ptr<ID3D11Texture2D1> texture;
    constexpr D3D11_TEXTURE2D_DESC1 desc {
      .Width = static_cast<DWORD>(Size.mWidth),
      .Height = static_cast<DWORD>(Size.mHeight),
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .SampleDesc = {1, 0},
      .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
      .MiscFlags
      = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED,
    };
    CheckHResult(device->CreateTexture2D1(&desc, nullptr, texture.put()));
    const auto resource = texture.query<IDXGIResource1>();
    CheckHResult(resource->CreateSharedHandle(
      nullptr, DXGI_SHARED_RESOURCE_READ, nullptr, mTexture.put()));

    wil::com_ptr<ID3D11RenderTargetView> rtv;
    CheckHResult(
      device->CreateRenderTargetView(texture.get(), nullptr, rtv.put()));

    static constexpr float Red[4] {1.f, 0.f, 0.f, 1.f};
    static constexpr float Green[4] {0.f, 1.f, 0.f, 1.f};
    static constexpr float White[4] {1.f, 1.f, 1.f, 1.f};

    while (!stop.stop_requested()) {
      const auto start = std::chrono::steady_clock::now();
      if (!mReady.test_and_set()) {
        mReady.notify_all();
      }
      ++mFenceValue;

      const auto offset
        = (std::chrono::duration_cast<std::chrono::milliseconds>(
             start.time_since_epoch())
             .count()
           / 10)
        % std::llround(Size.mWidth);
      const D3D11_RECT line {
        static_cast<LONG>(offset),
        0,
        static_cast<LONG>(offset + 1),
        static_cast<LONG>(Size.mHeight),
      };

      context->ClearRenderTargetView(rtv.get(), Red);
      context->ClearView(rtv.get(), White, &line, 1);
      if (mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      context->ClearRenderTargetView(rtv.get(), Green);
      context->ClearView(rtv.get(), White, &line, 1);
      if (!mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }

      const auto elapsed = std::chrono::steady_clock::now() - start;
      std::this_thread::sleep_for(std::chrono::milliseconds(100) - elapsed);
    }
  }
};
struct SwapChainPusher {
  fui::Widgets::SwapChainPanel::SwapChain mSwapChain;
  wil::unique_handle mFence {};
  std::atomic<uint64_t> mFenceValue {};
  bool mEarlySignal {false};

  static constexpr fui::Size Size {128, 128};

  SwapChainPusher(fui::Widgets::SwapChainPanel::SwapChain swapChain)
    : mSwapChain(std::move(swapChain)) {
    mThread = std::jthread {std::bind_front(&SwapChainPusher::Run, this)};
  }

 private:
  std::jthread mThread;

  void Run(const std::stop_token& stop) {
    using fui::win32_detail::CheckHResult;
    wil::com_ptr<ID3D11Device> basicDevice;
    wil::com_ptr<ID3D11DeviceContext> basicContext;
    CheckHResult(D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
        | D3D11_CREATE_DEVICE_DEBUG
#endif
      ,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      basicDevice.put(),
      nullptr,
      basicContext.put()));
    const auto device = basicDevice.query<ID3D11Device5>();
    const auto context = basicContext.query<ID3D11DeviceContext4>();

    wil::com_ptr<ID3D11Fence> fence;
    CheckHResult(device->CreateFence(
      0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(fence.put())));
    CheckHResult(
      fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, mFence.put()));
    bool fenceIsNew = true;

    static constexpr float Red[4] {1.f, 0.f, 0.f, 1.f};
    static constexpr float Green[4] {0.f, 1.f, 0.f, 1.f};
    static constexpr float White[4] {1.f, 1.f, 1.f, 1.f};
    while (!stop.stop_requested()) {
      const auto begin = mSwapChain.BeginFrame();
      if (!begin) {
        return;
      }

      const auto start = std::chrono::steady_clock::now();
      wil::com_ptr<ID3D11Texture2D> texture;
      CheckHResult(device->OpenSharedResource1(
        begin->mTexture, IID_PPV_ARGS(texture.put())));
      wil::com_ptr<ID3D11RenderTargetView> rtv;
      CheckHResult(
        device->CreateRenderTargetView(texture.get(), nullptr, rtv.put()));

      ++mFenceValue;

      const auto offset
        = (std::chrono::duration_cast<std::chrono::milliseconds>(
             start.time_since_epoch())
             .count()
           / 10)
        % std::llround(Size.mWidth);
      const D3D11_RECT line {
        static_cast<LONG>(offset),
        0,
        static_cast<LONG>(offset + 1),
        static_cast<LONG>(Size.mHeight),
      };

      context->ClearRenderTargetView(rtv.get(), Red);
      context->ClearView(rtv.get(), White, &line, 1);
      if (mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      context->ClearRenderTargetView(rtv.get(), Green);
      context->ClearView(rtv.get(), White, &line, 1);
      if (!mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }

      const fui::Widgets::SwapChainPanel::SwapChain::EndFrameInfo end {
        .mFence = mFence.get(),
        .mFenceValue = mFenceValue,
        .mFenceIsNew = std::exchange(fenceIsNew, false),
      };
      mSwapChain.EndFrame(*begin, end);

      const auto elapsed = std::chrono::steady_clock::now() - start;
      std::this_thread::sleep_for(std::chrono::milliseconds(100) - elapsed);
    }
  }
};
}// namespace

void demo_win32() {
  static TextureProducer textureSource;
  textureSource.WaitUntilReady();
  fuii::ToggleSwitch(&textureSource.mEarlySignal).Caption("Early fence signal");
  fuii::GPUTexture(
    fui::ImportedTexture::HandleKind::NTHandle,
    textureSource.mTexture.get(),
    textureSource.mFence.get(),
    textureSource.mFenceValue,
    fui::Rect {fui::Point {}, TextureProducer::Size})
    .Styled(
      fui::Style()
        .Width(TextureProducer::Size.mWidth)
        .Height(TextureProducer::Size.mHeight));

  fuii::SwapChainPanel([](const auto& swapChain) {
    static SwapChainPusher swapChainPusher {swapChain};
    swapChainPusher.mEarlySignal = textureSource.mEarlySignal;
  })
    .Styled(
      fui::Style()
        .Width(SwapChainPusher::Size.mWidth)
        .Height(SwapChainPusher::Size.mHeight));
}