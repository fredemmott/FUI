// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include <utility>

namespace FredEmmott::GUI::win32_detail {

template <class... Ts>
class COMImplementation : public Ts... {
 public:
  virtual ~COMImplementation() = default;

  ULONG AddRef() override {
    return InterlockedIncrement(&mRefCount);
  }

  ULONG Release() override {
    const auto count = InterlockedDecrement(&mRefCount);
    if (count == 0) {
      delete this;
    }
    return count;
  }

  HRESULT QueryInterface(const IID& riid, void** ppvObject) override {
    if (riid == __uuidof(IUnknown)) {
      *ppvObject = static_cast<IUnknown*>(this);
      AddRef();
      return S_OK;
    }

    if ((QueryInterfaceImpl<Ts>(riid, ppvObject) || ...)) {
      return S_OK;
    }
    return E_NOINTERFACE;
  }

 private:
  ULONG mRefCount {};

  template <class T>
  bool QueryInterfaceImpl(const IID& riid, void** ppvObject) {
    if (riid != __uuidof(T)) {
      return false;
    }
    *ppvObject = static_cast<T*>(this);
    AddRef();
    return true;
  }
};

}// namespace FredEmmott::GUI::win32_detail