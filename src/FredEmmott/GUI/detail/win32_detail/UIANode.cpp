// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "UIANode.hpp"

#include <Windows.h>
#include <yoga/Yoga.h>

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/Windows/Win32Window.hpp>
#include <format>

#include "UIARoot.hpp"

namespace FredEmmott::GUI::win32_detail {

UIANode::UIANode(Win32Window* w, Widgets::Widget* widget, UIARoot* root)
  : mWindow(w),
    mWidget(widget),
    mRoot(root) {}

// IRawElementProviderSimple
HRESULT STDMETHODCALLTYPE
UIANode::get_ProviderOptions(ProviderOptions* pRetVal) {
  *pRetVal = ProviderOptions_ServerSideProvider;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIANode::GetPatternProvider(PATTERNID, IUnknown** pRetVal) {
  *pRetVal = nullptr;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIANode::GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal) {
  VariantInit(pRetVal);
  if (propertyId == UIA_ControlTypePropertyId) {
    pRetVal->vt = VT_I4;
    pRetVal->lVal = UIA_CustomControlTypeId;
    return S_OK;
  }
  if (propertyId == UIA_NamePropertyId) {
    pRetVal->vt = VT_BSTR;
    auto name = std::format(L"Widget#{}", mWidget->GetID());
    pRetVal->bstrVal = SysAllocString(name.c_str());
    return S_OK;
  }
  pRetVal->vt = VT_EMPTY;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIANode::get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) {
  *pRetVal = nullptr;
  return S_OK;
}

// IRawElementProviderFragment
HRESULT STDMETHODCALLTYPE UIANode::Navigate(
  NavigateDirection direction,
  IRawElementProviderFragment** pRetVal) {
  *pRetVal = nullptr;
  switch (direction) {
    case NavigateDirection_Parent:
      if (auto parent = mWidget->GetParent()) {
        *pRetVal = new UIANode(mWindow, parent, mRoot);
      } else {
        *pRetVal = mRoot;
        mRoot->AddRef();
      }
      return S_OK;
    case NavigateDirection_FirstChild: {
      const auto children = mWidget->GetChildren();
      if (!children.empty()) {
        *pRetVal = new UIANode(mWindow, children.front(), mRoot);
      }
      return S_OK;
    }
    case NavigateDirection_LastChild: {
      const auto children = mWidget->GetChildren();
      if (!children.empty()) {
        *pRetVal = new UIANode(mWindow, children.back(), mRoot);
      }
      return S_OK;
    }
    case NavigateDirection_NextSibling: {
      auto parent = mWidget->GetParent();
      if (!parent)
        return S_OK;
      const auto children = parent->GetChildren();
      for (size_t i = 0; i + 1 < children.size(); ++i) {
        if (children[i] == mWidget) {
          *pRetVal = new UIANode(mWindow, children[i + 1], mRoot);
          break;
        }
      }
      return S_OK;
    }
    case NavigateDirection_PreviousSibling: {
      auto parent = mWidget->GetParent();
      if (!parent)
        return S_OK;
      const auto children = parent->GetChildren();
      for (size_t i = 1; i < children.size(); ++i) {
        if (children[i] == mWidget) {
          *pRetVal = new UIANode(mWindow, children[i - 1], mRoot);
          break;
        }
      }
      return S_OK;
    }
    default:
      return S_OK;
  }
}

HRESULT STDMETHODCALLTYPE UIANode::GetRuntimeId(SAFEARRAY** pRetVal) {
  const int RID[] = {UiaAppendRuntimeId, 2, static_cast<int>(mWidget->GetID())};
  SAFEARRAY* psa = SafeArrayCreateVector(VT_I4, 0, ARRAYSIZE(RID));
  if (!psa)
    return E_OUTOFMEMORY;
  for (LONG i = 0; i < static_cast<LONG>(ARRAYSIZE(RID)); ++i) {
    SafeArrayPutElement(psa, &i, (void*)&RID[i]);
  }
  *pRetVal = psa;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE UIANode::get_BoundingRectangle(UiaRect* pRetVal) {
  const auto yoga = mWidget->GetLayoutNode();
  const float width = YGNodeLayoutGetWidth(yoga);
  const float height = YGNodeLayoutGetHeight(yoga);

  const auto topLeftCanvas = mWidget->GetTopLeftCanvasPoint();
  const auto [left, top]
    = mWindow->CanvasPointToNativePoint(topLeftCanvas).as<BasicPoint<double>>();
  const auto [right, bottom]
    = mWindow->CanvasPointToNativePoint(topLeftCanvas + Size {width, height})
        .as<BasicPoint<double>>();
  *pRetVal = {left, top, right - left, bottom - top};
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIANode::GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal) {
  *pRetVal = nullptr;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE UIANode::SetFocus() {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
UIANode::get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal) {
  *pRetVal = mRoot;
  mRoot->AddRef();
  return S_OK;
}

}// namespace FredEmmott::GUI::win32_detail
