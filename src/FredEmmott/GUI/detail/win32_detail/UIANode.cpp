// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "UIANode.hpp"

#include <Windows.h>
#include <yoga/Yoga.h>

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/Windows/Win32Window.hpp>
#include <format>

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "UIARoot.hpp"

namespace FredEmmott::GUI::win32_detail {

namespace {

using Widget = Widgets::Widget;

[[nodiscard]]
bool IsDisplayContents(Widget* widget) {
  const auto yoga = widget->GetLayoutNode();
  return YGNodeStyleGetDisplay(yoga) == YGDisplayContents;
}

// Flattens display: contents
void GetFlattenedChildren(
  const Widget* widget,
  std::vector<Widgets::Widget*>& out) {
  out.reserve(out.size() + widget->GetChildren().size());

  for (auto&& child: widget->GetChildren()) {
    if (!IsDisplayContents(child)) {
      out.emplace_back(child);
      continue;
    }
    GetFlattenedChildren(child, out);
  }
}

// Flattens display: contents
Widget* GetFlattenedParent(Widget* widget) {
  while (((widget = widget->GetParent())) && IsDisplayContents(widget)) {
  }
  return widget;
}

std::vector<Widget*> GetFlattenedChildren(const Widget* widget) {
  std::vector<Widget*> out;
  GetFlattenedChildren(widget, out);
  return out;
}

}// namespace

UIANode::UIANode(Win32Window* w, Widgets::Widget* widget, UIARoot* root)
  : mWindow(w),
    mWidget(widget),
    mRoot(root) {
  if (!mWidget) {
    __debugbreak();
  }
}

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
  if (propertyId == UIA_IsOffscreenPropertyId) {
    pRetVal->vt = VT_BOOL;
    pRetVal->boolVal
      = YGNodeStyleGetDisplay(mWidget->GetLayoutNode()) == YGDisplayNone;
    return S_OK;
  }
  if (propertyId == UIA_ControlTypePropertyId) {
    pRetVal->vt = VT_I4;
    pRetVal->lVal = UIA_CustomControlTypeId;
    return S_OK;
  }
  if (propertyId == UIA_NamePropertyId) {
    pRetVal->vt = VT_BSTR;
    auto name = Utf8ToWide(mWidget->GetAccessibilityName());
    pRetVal->bstrVal = SysAllocString(name.c_str());
    return S_OK;
  }
  if (propertyId == UIA_BoundingRectanglePropertyId) {
    UiaRect rect {};
    CheckHResult(get_BoundingRectangle(&rect));

    pRetVal->vt = VT_ARRAY | VT_R8;
    pRetVal->parray = SafeArrayCreateVector(VT_R8, 0, 4);

    double* data {};
    SafeArrayAccessData(pRetVal->parray, reinterpret_cast<void**>(&data));
    data[0] = rect.left;
    data[1] = rect.top;
    data[2] = rect.width;
    data[3] = rect.height;
    SafeArrayUnaccessData(pRetVal->parray);
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
  const auto addRef = wil::scope_exit([&]() {
    if (*pRetVal) {
      (*pRetVal)->AddRef();
    }
  });

  switch (direction) {
    case NavigateDirection_Parent:
      if (auto parent = GetFlattenedParent(mWidget)) {
        *pRetVal = new UIANode(mWindow, parent, mRoot);
      } else {
        *pRetVal = mRoot;
      }
      return S_OK;
    case NavigateDirection_FirstChild: {
      const auto children = GetFlattenedChildren(mWidget);
      if (!children.empty()) {
        *pRetVal = new UIANode(mWindow, children.front(), mRoot);
      }
      return S_OK;
    }
    case NavigateDirection_LastChild: {
      const auto children = GetFlattenedChildren(mWidget);
      if (!children.empty()) {
        *pRetVal = new UIANode(mWindow, children.back(), mRoot);
      }
      return S_OK;
    }
    case NavigateDirection_PreviousSibling: {
      const auto parent = GetFlattenedParent(mWidget);
      if (!parent)
        return S_OK;
      const auto children = GetFlattenedChildren(parent);
      auto it = std::ranges::find(children, mWidget);
      if (it == children.end() || it == children.begin()) {
        return S_OK;
      }
      *pRetVal = new UIANode(mWindow, *--it, mRoot);
      return S_OK;
    }
    case NavigateDirection_NextSibling: {
      const auto parent = GetFlattenedParent(mWidget);
      if (!parent)
        return S_OK;
      const auto children = GetFlattenedChildren(parent);
      auto it = std::ranges::find(children, mWidget);
      if (it == children.end()) {
        return S_OK;
      }
      if (++it == children.end()) {
        return S_OK;
      }

      *pRetVal = new UIANode(mWindow, *it, mRoot);
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
