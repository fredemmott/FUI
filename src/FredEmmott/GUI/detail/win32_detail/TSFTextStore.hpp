// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>
#include <msctf.h>
#include <textstor.h>
#include <wil/com.h>

#include <string>

#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail/COMImplementation.hpp"

namespace FredEmmott::GUI::Widgets {
class TextBox;
}

namespace FredEmmott::GUI::win32_detail {

// Minimal ITextStoreACP2 implementation that proxies a TextBox's text and
// selection.
class TextStoreACP final : public COMImplementation<ITextStoreACP2> {
 public:
  explicit TextStoreACP(Widgets::TextBox* owner);

  // IUnknown via COMImplementation

  // ITextStoreACP
  HRESULT
  AdviseSink(REFIID riid, IUnknown* punk, DWORD dwMask) override;
  HRESULT UnadviseSink(IUnknown* punk) override;
  HRESULT
  RequestLock(DWORD dwLockFlags, HRESULT* phrSession) override;
  HRESULT GetStatus(TS_STATUS* pdcs) override;
  HRESULT QueryInsert(
    LONG acpTestStart,
    LONG acpTestEnd,
    ULONG cch,
    LONG* pacpResultStart,
    LONG* pacpResultEnd) override;
  HRESULT GetSelection(
    ULONG ulIndex,
    ULONG ulCount,
    TS_SELECTION_ACP* pSelection,
    ULONG* pcFetched) override;
  HRESULT
  SetSelection(ULONG ulCount, const TS_SELECTION_ACP* pSelection) override;
  HRESULT GetText(
    LONG acpStart,
    LONG acpEnd,
    WCHAR* pchPlain,
    ULONG cchPlainReq,
    ULONG* pcchPlainOut,
    TS_RUNINFO* prgRunInfo,
    ULONG cRunInfoReq,
    ULONG* pcRunInfoOut,
    LONG* pacpNext) override;
  HRESULT SetText(
    DWORD dwFlags,
    LONG acpStart,
    LONG acpEnd,
    const WCHAR* pchText,
    ULONG cch,
    TS_TEXTCHANGE* pChange) override;
  HRESULT GetEndACP(LONG* pacp) override;
  HRESULT GetActiveView(TsViewCookie* pvcView) override;
  HRESULT GetACPFromPoint(
    TsViewCookie vcView,
    const POINT* pt,
    DWORD dwFlags,
    LONG* pacp) override;
  HRESULT GetTextExt(
    TsViewCookie vcView,
    LONG acpStart,
    LONG acpEnd,
    RECT* prc,
    BOOL* pfClipped) override;
  HRESULT
  GetScreenExt(TsViewCookie vcView, RECT* prc) override;
  HRESULT
  InsertEmbedded(DWORD, LONG, LONG, IDataObject*, TS_TEXTCHANGE*) override {
    return TS_E_FORMAT;
  }
  HRESULT
  InsertEmbeddedAtSelection(DWORD, IDataObject*, LONG*, LONG*, TS_TEXTCHANGE*)
    override {
    return TS_E_FORMAT;
  }
  HRESULT
  RequestSupportedAttrs(DWORD, ULONG, const TS_ATTRID*) override {
    return S_OK;
  }
  HRESULT
  RequestAttrsAtPosition(LONG, ULONG, const TS_ATTRID*, DWORD) override {
    return S_OK;
  }
  HRESULT RequestAttrsTransitioningAtPosition(
    LONG,
    ULONG,
    const TS_ATTRID*,
    DWORD) override {
    return S_OK;
  }
  HRESULT FindNextAttrTransition(
    LONG,
    LONG,
    ULONG,
    const TS_ATTRID*,
    DWORD,
    LONG*,
    BOOL*,
    LONG*) override {
    return TS_E_NOLOCK;
  }
  HRESULT
  RetrieveRequestedAttrs(ULONG, TS_ATTRVAL*, ULONG*) override {
    return S_OK;
  }
  HRESULT
  GetEmbedded(LONG, REFGUID, REFIID, IUnknown**) override {
    return TS_E_NOLAYOUT;
  }
  HRESULT
  QueryInsertEmbedded(const GUID*, const FORMATETC*, BOOL*) override {
    return S_OK;
  }
  HRESULT InsertTextAtSelection(
    DWORD dwFlags,
    const WCHAR* pchText,
    ULONG cch,
    LONG* pacpStart,
    LONG* pacpEnd,
    TS_TEXTCHANGE* pChange) override;

  ITextStoreACPSink* GetSink(const DWORD mask) const {
    if (!mSink) {
      return nullptr;
    }
    if (!(mSinkMask & mask)) {
      return nullptr;
    }
    return mSink.get();
  }

 private:
  Widgets::TextBox* mOwner {};
  wil::com_ptr_nothrow<ITextStoreACPSink> mSink;
  DWORD mSinkMask {};
  bool mLocked {false};

  std::wstring GetWideText() const;

 public:
  HRESULT GetFormattedText(
    LONG acpStart,
    LONG acpEnd,
    IDataObject** ppDataObject) override;
};

// Per-thread TSF manager
class TSFThreadManager final {
 public:
  static TSFThreadManager& Get();

  void Initialize(HWND hwnd);
  void Uninitialize();

  // Activate a document for a TextBox (creates doc mgr and context)
  struct Document {
    wil::com_ptr_nothrow<ITfDocumentMgr> mDocMgr;
    wil::com_ptr_nothrow<ITfContext> mContext;
    wil::com_ptr_nothrow<TextStoreACP> mStore;
  };
  Document ActivateFor(HWND hwnd, Widgets::TextBox* owner);
  void Deactivate(Document& doc);

 private:
  TSFThreadManager() = default;
  wil::com_ptr_nothrow<ITfThreadMgr> mThreadMgr;
  TfClientId mClientId {};
  bool mInitialized {false};
};

}// namespace FredEmmott::GUI::win32_detail
