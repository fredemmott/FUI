// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "UIARoot.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/Windows/Win32Window.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>

#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "UIANode.hpp"

namespace FredEmmott::GUI::win32_detail {

UIARoot::UIARoot(Win32Window* w) : mWindow(w) {}

// IRawElementProviderSimple
HRESULT STDMETHODCALLTYPE
UIARoot::get_ProviderOptions(ProviderOptions* pRetVal) {
  *pRetVal = ProviderOptions_ServerSideProvider;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIARoot::GetPatternProvider(PATTERNID, IUnknown** pRetVal) {
  *pRetVal = nullptr;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIARoot::GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal) {
  VariantInit(pRetVal);
  if (propertyId == UIA_ControlTypePropertyId) {
    pRetVal->vt = VT_I4;
    pRetVal->lVal = UIA_WindowControlTypeId;
    return S_OK;
  }
  if (propertyId == UIA_NamePropertyId) {
    pRetVal->vt = VT_BSTR;
    const auto title = Utf8ToWide(std::string {mWindow->GetTitle()});
    pRetVal->bstrVal = SysAllocString(title.c_str());
    return S_OK;
  }
  if (propertyId == UIA_NativeWindowHandlePropertyId) {
    pRetVal->vt = VT_I4;
    pRetVal->lVal = PtrToLong(mWindow->GetNativeHandle());
    return S_OK;
  }
  pRetVal->vt = VT_EMPTY;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIARoot::get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) {
  return UiaHostProviderFromHwnd(
    static_cast<HWND>(mWindow->GetNativeHandle()), pRetVal);
}

// IRawElementProviderFragment
HRESULT STDMETHODCALLTYPE UIARoot::Navigate(
  NavigateDirection direction,
  IRawElementProviderFragment** pRetVal) {
  *pRetVal = nullptr;
  if (direction == NavigateDirection_FirstChild) {
    auto root = mWindow->GetRootWidget();
    if (!root)
      return S_OK;
    *pRetVal = new UIANode(mWindow, root, this);
    return S_OK;
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE UIARoot::GetRuntimeId(SAFEARRAY** pRetVal) {
  // Return provider-defined root runtime ID
  const int RID[] = {UiaAppendRuntimeId, 1, 0};
  SAFEARRAY* psa = SafeArrayCreateVector(VT_I4, 0, ARRAYSIZE(RID));
  if (!psa)
    return E_OUTOFMEMORY;
  for (LONG i = 0; i < static_cast<LONG>(ARRAYSIZE(RID)); ++i) {
    SafeArrayPutElement(psa, &i, (void*)&RID[i]);
  }
  *pRetVal = psa;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE UIARoot::get_BoundingRectangle(UiaRect* pRetVal) {
  RECT rc {};
  GetClientRect(static_cast<HWND>(mWindow->GetNativeHandle()), &rc);
  POINT origin {0, 0};
  ClientToScreen(static_cast<HWND>(mWindow->GetNativeHandle()), &origin);
  pRetVal->left = origin.x;
  pRetVal->top = origin.y;
  pRetVal->width = rc.right - rc.left;
  pRetVal->height = rc.bottom - rc.top;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIARoot::GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal) {
  *pRetVal = nullptr;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE UIARoot::SetFocus() {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIARoot::get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal) {
  *pRetVal = this;
  AddRef();
  return S_OK;
}

// IRawElementProviderFragmentRoot
HRESULT STDMETHODCALLTYPE UIARoot::ElementProviderFromPoint(
  double x,
  double y,
  IRawElementProviderFragment** pRetVal) {
  if (!pRetVal)
    return E_INVALIDARG;
  const auto& window = *Immediate::immediate_detail::tWindow;
  MouseEvent event;
  event.mWindowPoint
    = window.NativePointToCanvasPoint(BasicPoint {x, y}.as<NativePoint>());
  event.mDetail = MouseEvent::HitTestEvent {};
  const auto widget = window.GetRootWidget()->DispatchEvent(event);
  *pRetVal = new UIANode(mWindow, widget, this);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIARoot::GetFocus(IRawElementProviderFragment** pRetVal) {
  // Return focused widget if any
  const auto fm = FocusManager::Get();
  if (fm) {
    if (const auto fw = fm->GetFocusedWidget()) {
      Widgets::Widget* w = std::get<0>(*fw);
      *pRetVal = new UIANode(mWindow, w, this);
      return S_OK;
    }
  }
  *pRetVal = this;
  AddRef();
  return S_OK;
}

}// namespace FredEmmott::GUI::win32_detail
