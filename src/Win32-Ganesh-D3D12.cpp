// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32-Ganesh-D3D12.hpp"

#include <skia/core/SkCanvas.h>
#include <skia/core/SkColorSpace.h>
#include <skia/core/SkImageInfo.h>
#include <skia/gpu/GrBackendSemaphore.h>
#include <skia/gpu/GrBackendSurface.h>
#include <skia/gpu/GrDirectContext.h>
#include <skia/gpu/d3d/GrD3DBackendContext.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>

#include <format>
#include <source_location>

static inline void CheckHResult(
  const HRESULT ret,
  const std::source_location& caller = std::source_location::current()) {
  if (SUCCEEDED(ret)) [[likely]] {
    return;
  }

  const std::error_code ec {ret, std::system_category()};

  const auto msg = std::format(
    "HRESULT failed: {:#010x} @ {} - {}:{}:{} - {}\n",
    std::bit_cast<uint32_t>(ret),
    caller.function_name(),
    caller.file_name(),
    caller.line(),
    caller.column(),
    ec.message());
  OutputDebugStringA(msg.c_str());
  throw std::system_error(ec);
}

void HelloSkiaWindow::InitializeSkia() {
  GrD3DBackendContext skiaD3DContext {};
  skiaD3DContext.fAdapter.retain(mDXGIAdapter.get());
  skiaD3DContext.fDevice.retain(mD3DDevice.get());
  skiaD3DContext.fQueue.retain(mD3DCommandQueue.get());
  mSkiaContext = GrDirectContext::MakeDirect3D(skiaD3DContext);
}
HelloSkiaWindow::HelloSkiaWindow(HINSTANCE instance) {
  gInstance = this;

  this->CreateNativeWindow(instance);
  this->InitializeD3D();
  this->InitializeSkia();
  this->CreateRenderTargets();
}

void HelloSkiaWindow::CreateNativeWindow(HINSTANCE instance) {
  const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
  const auto height = screenHeight / 2;
  const auto width = (height * 2) / 3;

  const WNDCLASSW wc {
    .lpfnWndProc = &WindowProc,
    .hInstance = instance,
    .lpszClassName = L"Hello Skia",
  };
  const auto classAtom = RegisterClassW(&wc);
  mHwnd.reset(CreateWindowExW(
    WS_EX_APPWINDOW | WS_EX_CLIENTEDGE,
    MAKEINTATOM(classAtom),
    L"Hello Skia",
    WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width,
    height,
    nullptr,
    nullptr,
    instance,
    nullptr));

  if (!mHwnd) {
    CheckHResult(HRESULT_FROM_WIN32(GetLastError()));
  }
}

void HelloSkiaWindow::InitializeD3D() {
#ifndef NDEBUG
  wil::com_ptr<ID3D12Debug> d3d12Debug;
  D3D12GetDebugInterface(IID_PPV_ARGS(d3d12Debug.put()));
  if (d3d12Debug) {
    d3d12Debug->EnableDebugLayer();
  }
#endif

  wil::com_ptr<IDXGIFactory4> dxgiFactory;
  {
    UINT flags = 0;
#ifndef NDEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    CheckHResult(CreateDXGIFactory2(flags, IID_PPV_ARGS(dxgiFactory.put())));
  }

  CheckHResult(dxgiFactory->EnumAdapters1(0, mDXGIAdapter.put()));

  D3D_FEATURE_LEVEL featureLevel {D3D_FEATURE_LEVEL_11_0};
  CheckHResult(D3D12CreateDevice(
    mDXGIAdapter.get(), featureLevel, IID_PPV_ARGS(mD3DDevice.put())));
  this->ConfigureD3DDebugLayer();

  CheckHResult(mD3DDevice->CreateFence(
    0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mD3DFence.put())));

  {
    D3D12_COMMAND_QUEUE_DESC desc {
      .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
      .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
    };
    CheckHResult(mD3DDevice->CreateCommandQueue(
      &desc, IID_PPV_ARGS(mD3DCommandQueue.put())));
  }

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc {
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
      .NumDescriptors = SwapChainLength,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };
    CheckHResult(
      mD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mD3DRTVHeap.put())));
  }

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc {
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
      .NumDescriptors = 1,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
    };
    CheckHResult(
      mD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mD3DSRVHeap.put())));
  }

  this->CreateCommandListAndAllocators();

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {1, 0},
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = SwapChainLength,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
  };

  CheckHResult(dxgiFactory->CreateSwapChainForHwnd(
    mD3DCommandQueue.get(),
    mHwnd.get(),
    &swapChainDesc,
    nullptr,
    nullptr,
    mSwapChain.put()));
  CheckHResult(mSwapChain->GetDesc1(&swapChainDesc));
  mWindowSize = {swapChainDesc.Width, swapChainDesc.Height};
}

void HelloSkiaWindow::CreateCommandListAndAllocators() {
  for (auto& frame: mFrames) {
    CheckHResult(mD3DDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT,
      IID_PPV_ARGS(frame.mCommandAllocator.put())));
  }
  // We'll reset this to the appropriate command allocator each frame
  CheckHResult(mD3DDevice->CreateCommandList(
    0,
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    mFrames.front().mCommandAllocator.get(),
    nullptr,
    IID_PPV_ARGS(mD3DCommandList.put())));
  CheckHResult(mD3DCommandList->Close());
}


void HelloSkiaWindow::ConfigureD3DDebugLayer() {
#ifndef NDEBUG
  auto infoQueue = mD3DDevice.try_query<ID3D12InfoQueue1>();
  if (!infoQueue) {
    return;
  }

  infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
  infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
  infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

  // Skia internally triggers this; explicitly suppress it so we can
  // keep breaking on everything WARNING or above
  D3D12_MESSAGE_ID skiaIssues[] = {
    D3D12_MESSAGE_ID_DESCRIPTOR_HEAP_NOT_SHADER_VISIBLE,
  };
  for (const auto id: skiaIssues) {
    infoQueue->SetBreakOnID(id, false);
  }

  D3D12_MESSAGE_SEVERITY allowSeverities[] = {
    D3D12_MESSAGE_SEVERITY_WARNING,
    D3D12_MESSAGE_SEVERITY_ERROR,
    D3D12_MESSAGE_SEVERITY_CORRUPTION,
  };

  D3D12_INFO_QUEUE_FILTER filter {
        .AllowList = D3D12_INFO_QUEUE_FILTER_DESC {
          .NumSeverities = std::size(allowSeverities),
          .pSeverityList = allowSeverities,
        },
        .DenyList = D3D12_INFO_QUEUE_FILTER_DESC {
          .NumIDs = std::size(skiaIssues),
          .pIDList = skiaIssues,
        },
      };
  CheckHResult(infoQueue->PushStorageFilter(&filter));
#endif
}

HelloSkiaWindow::~HelloSkiaWindow() {
  if (mFenceValue) {
    CheckHResult(
      mD3DFence->SetEventOnCompletion(mFenceValue, mFenceEvent.get()));
    WaitForSingleObject(mFenceEvent.get(), INFINITE);
  }
  gInstance = nullptr;
}

void HelloSkiaWindow::CreateRenderTargets() {
  const auto rtvStart = mD3DRTVHeap->GetCPUDescriptorHandleForHeapStart();
  const auto rtvStep = mD3DDevice->GetDescriptorHandleIncrementSize(
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  for (UINT i = 0; i < SwapChainLength; ++i) {
    auto& frame = mFrames[i];
    CheckHResult(
      mSwapChain->GetBuffer(i, IID_PPV_ARGS(frame.mRenderTarget.put())));
    frame.mRenderTargetView = rtvStart;
    frame.mRenderTargetView.ptr += i * rtvStep;
    mD3DDevice->CreateRenderTargetView(
      frame.mRenderTarget.get(), nullptr, frame.mRenderTargetView);

    DXGI_SWAP_CHAIN_DESC1 desc;
    mSwapChain->GetDesc1(&desc);

    frame.mRenderTarget->AddRef();
    GrD3DTextureResourceInfo backBufferInfo(
      frame.mRenderTarget.get(),
      {},
      D3D12_RESOURCE_STATE_RENDER_TARGET,
      DXGI_FORMAT_R8G8B8A8_UNORM,
      1,
      1,
      0);
    GrBackendRenderTarget backBufferRT(
      static_cast<int>(desc.Width),
      static_cast<int>(desc.Height),
      backBufferInfo);
    frame.mSkSurface = SkSurfaces::WrapBackendRenderTarget(
      mSkiaContext.get(),
      backBufferRT,
      {},
      kRGBA_8888_SkColorType,
      nullptr,
      nullptr);
  }
}

HWND HelloSkiaWindow::GetHWND() const noexcept {
  return mHwnd.get();
}

int HelloSkiaWindow::Run() noexcept {
  while (!mExitCode) {
    MSG msg {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        return mExitCode.value_or(0);
      }
    }

    if (mPendingResize) {
      this->CleanupFrameContexts();
      CheckHResult(mSwapChain->ResizeBuffers(
        0,
        mPendingResize->mWidth,
        mPendingResize->mHeight,
        DXGI_FORMAT_UNKNOWN,
        0));
      this->CreateRenderTargets();

      mWindowSize = *mPendingResize;
      mPendingResize = std::nullopt;
    }

    const auto frameNumber = mFrameCounter++;
    auto& frame = mFrames.at(frameNumber % SwapChainLength);

    auto commandList = mD3DCommandList.get();
    if (frame.mFenceValue) {
      mD3DFence->SetEventOnCompletion(frame.mFenceValue, mFenceEvent.get());
      WaitForSingleObject(mFenceEvent.get(), INFINITE);
    }
    CheckHResult(frame.mCommandAllocator->Reset());
    frame.mFenceValue = ++mFenceValue;
    CheckHResult(commandList->Reset(frame.mCommandAllocator.get(), nullptr));

    D3D12_RESOURCE_BARRIER barrier {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Transition = D3D12_RESOURCE_TRANSITION_BARRIER {
          .pResource = frame.mRenderTarget.get(),
          .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
          .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
          .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
        },
      };
    commandList->ResourceBarrier(1, &barrier);

    FLOAT clearColor[4] {0.0f, 0.0f, 0.0f, 1.0f};
    commandList->ClearRenderTargetView(
      frame.mRenderTargetView, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(
      1, &frame.mRenderTargetView, FALSE, nullptr);
    {
      auto ptr = mD3DSRVHeap.get();
      commandList->SetDescriptorHeaps(1, &ptr);
    }
    CheckHResult(commandList->Close());

    {
      auto upcast = static_cast<ID3D12CommandList*>(commandList);
      mD3DCommandQueue->ExecuteCommandLists(1, &upcast);
      CheckHResult(
        mD3DCommandQueue->Signal(mD3DFence.get(), frame.mFenceValue));
    }

    {
      // Skia Test
      GrD3DFenceInfo fenceInfo;
      fenceInfo.fFence.retain(mD3DFence.get());
      fenceInfo.fValue = frame.mFenceValue;
      GrBackendSemaphore imguiFence;
      imguiFence.initDirect3D(fenceInfo);
      mSkiaContext->wait(1, &imguiFence, false);
      auto brt = SkSurfaces::GetBackendRenderTarget(
        frame.mSkSurface.get(), SkSurfaces::BackendHandleAccess::kFlushWrite);
      brt.setD3DResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET);

      auto canvas = frame.mSkSurface->getCanvas();

      SkPaint paint;
      paint.setColor(SkColorSetARGB(0x80, 0xff, 0xff, 0xff));
      paint.setStyle(SkPaint::kStrokeAndFill_Style);

      canvas->drawCircle(SkPoint {20.0f, 20.0f}, 20.0f, paint);
      mSkiaContext->flush(
        frame.mSkSurface.get(),
        SkSurfaces::BackendSurfaceAccess::kPresent,
        GrFlushInfo {});
      mSkiaContext->submit(GrSyncCpu::kNo);
      frame.mFenceValue = ++mFenceValue;
      CheckHResult(
        mD3DCommandQueue->Signal(mD3DFence.get(), frame.mFenceValue));
    }

    CheckHResult(mSwapChain->Present(1, 0));

    WaitMessage();
  }

  mD3DFence->SetEventOnCompletion(mFenceValue, mFenceEvent.get());
  WaitForSingleObject(mFenceEvent.get(), INFINITE);
  this->CleanupFrameContexts();

  return *mExitCode;
}

LRESULT
HelloSkiaWindow::WindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam) noexcept {
  if (uMsg == WM_SIZE) {
    const UINT width = LOWORD(lParam);
    const UINT height = HIWORD(lParam);
    gInstance->mPendingResize = PixelSize {width, height};
    return 0;
  }
  if (uMsg == WM_CLOSE) {
    gInstance->mExitCode = 0;
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void HelloSkiaWindow::CleanupFrameContexts() {
  CheckHResult(mD3DFence->SetEventOnCompletion(mFenceValue, mFenceEvent.get()));
  WaitForSingleObject(mFenceEvent.get(), INFINITE);
  for (auto& frame: mFrames) {
    frame.mSkSurface = {};
    frame.mRenderTarget = nullptr;
    frame.mRenderTargetView = {};
  }
}

HelloSkiaWindow* HelloSkiaWindow::gInstance {nullptr};

int WINAPI wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR lpCmdLine,
  int nCmdShow) {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  HelloSkiaWindow app(hInstance);
  ShowWindow(app.GetHWND(), nCmdShow);
  return app.Run();
}
