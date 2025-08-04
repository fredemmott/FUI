// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.ApplicationModel.Activation.h>

#include <functional>

#include "COMImplementation.hpp"

namespace FredEmmott::GUI::win32_detail {

// Not using C++/WinRT because that makes it so that all statically-linked
// consumers must use the exact same version of C++/WinRT, even when it's just
// an implementation detail; given that it's available in the Windows
// SDK, vcpkg, and nuget (the only officially supported method), using
// C++/WinRT in a static library seems likely to cause significant user pain,
// and it's easier to just user the ABI directly where we need it
template <class TSender, class TSenderABI, class TArg, class TArgABI>
class WinRTEventHandler
  : public COMImplementation<
      ABI::Windows::Foundation::ITypedEventHandler<TSender*, TArg*>> {
 public:
  using callback_t = std::function<HRESULT(TSenderABI*, TArgABI*)>;

  WinRTEventHandler() = default;
  explicit WinRTEventHandler(callback_t callback) : mCallback(callback) {}

  WinRTEventHandler& operator=(callback_t callback) {
    mCallback = callback;
    return *this;
  }

  HRESULT Invoke(TSenderABI* sender, TArgABI* arg) final {
    if (mCallback) {
      return mCallback(sender, arg);
    }
    return S_OK;
  }

 private:
  callback_t mCallback;
};

}// namespace FredEmmott::GUI::win32_detail