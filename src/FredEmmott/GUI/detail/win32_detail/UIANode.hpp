// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <UIAutomationClient.h>
#include <UIAutomationCoreApi.h>

#include <FredEmmott/GUI/detail/win32_detail/COMImplementation.hpp>

namespace FredEmmott::GUI {
class Win32Window;
namespace Widgets {
class Widget;
}
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::win32_detail {

struct UIARoot;

struct UIANode final
  : COMImplementation<IRawElementProviderSimple, IRawElementProviderFragment> {
  UIANode() = delete;
  UIANode(Win32Window* w, Widgets::Widget* widget, UIARoot* root);

  // IRawElementProviderSimple
  HRESULT STDMETHODCALLTYPE
  get_ProviderOptions(ProviderOptions* pRetVal) override;
  HRESULT STDMETHODCALLTYPE
  GetPatternProvider(PATTERNID patternId, IUnknown** pRetVal) override;
  HRESULT STDMETHODCALLTYPE
  GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal) override;
  HRESULT STDMETHODCALLTYPE
  get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) override;

  // IRawElementProviderFragment
  HRESULT STDMETHODCALLTYPE Navigate(
    NavigateDirection direction,
    IRawElementProviderFragment** pRetVal) override;
  HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** pRetVal) override;
  HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* pRetVal) override;
  HRESULT STDMETHODCALLTYPE
  GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal) override;
  HRESULT STDMETHODCALLTYPE SetFocus() override;
  HRESULT STDMETHODCALLTYPE
  get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal) override;

  Win32Window* mWindow {};
  Widgets::Widget* mWidget {};
  UIARoot* mRoot {};
};

}// namespace FredEmmott::GUI::win32_detail
