// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TSFTextStore.hpp"

#include <olectl.h>
#include <wil/com.h>
#include <wil/resource.h>

#include <FredEmmott/GUI/Widgets/TextBox.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>
#include <algorithm>
#include <print>

namespace FredEmmott::GUI::win32_detail {

using namespace Widgets;

TextStoreACP::TextStoreACP(TextBox* owner) : mOwner(owner) {}

HRESULT TextStoreACP::GetWnd(TsViewCookie, HWND* phwnd) {
  if (!phwnd) {
    return E_INVALIDARG;
  }
  *phwnd = mOwner->GetOwnerWindow()->GetNativeHandle().mValue;
  return S_OK;
}

HRESULT
TextStoreACP::AdviseSink(REFIID riid, IUnknown* punk, const DWORD dwMask) {
  if (riid != __uuidof(ITextStoreACPSink)) {
    return E_INVALIDARG;
  }

  if (mSink) {
    return CONNECT_E_ADVISELIMIT;
  }

  mSink = wil::com_query<ITextStoreACPSink>(punk);
  if (!mSink) {
    return E_UNEXPECTED;
  }
  mSinkMask = dwMask;

  if (dwMask & TS_AS_LAYOUT_CHANGE)
    CheckHResult(mSink->OnLayoutChange(TS_LC_CREATE, 1));

  return S_OK;
}

HRESULT TextStoreACP::UnadviseSink(IUnknown* punk) {
  if ((!mSink) || wil::com_query<ITextStoreACPSink>(punk) != mSink) {
    return CONNECT_E_NOCONNECTION;
  }
  mSink.reset();
  mSinkMask = 0;
  return S_OK;
}

HRESULT TextStoreACP::RequestLock(DWORD dwLockFlags, HRESULT* phrSession) {
  FUI_ASSERT(
    mOwnerThread == std::this_thread::get_id(),
    "Must be called on owner thread");

  if (!phrSession)
    return E_INVALIDARG;
  if (mLockFlags) {
    const auto lockedMode = (mLockFlags & TS_LF_READWRITE);
    const auto wantMode = (dwLockFlags & TS_LF_READWRITE);
    if ((lockedMode & wantMode) == wantMode) {
      const auto oldFlags = std::exchange(mLockFlags, dwLockFlags);
      const auto restore
        = wil::scope_exit([this, oldFlags] { mLockFlags = oldFlags; });
      *phrSession = mSink->OnLockGranted(dwLockFlags);
      return S_OK;
    }

    if ((dwLockFlags & TS_LF_SYNC) == TS_LF_SYNC) {
      *phrSession = TS_E_SYNCHRONOUS;
      return S_OK;
    }

    *phrSession = TS_S_ASYNC;
    mPendingLocks.push(dwLockFlags);
    return S_OK;
  }

  if (!mSink) {
    *phrSession = S_OK;
    return S_OK;
  }

  const auto restore
    = wil::scope_exit([this, oldFlags = mLockFlags] { mLockFlags = oldFlags; });

  mPendingLocks.push(dwLockFlags);
  bool isFirst {true};
  while (!mPendingLocks.empty()) {
    mLockFlags = mPendingLocks.front();
    mPendingLocks.pop();

    const auto hr = mSink->OnLockGranted(mLockFlags);
    if (std::exchange(isFirst, false)) {
      *phrSession = hr;
    }

    if (FAILED(hr)) {
      return hr;
    }
  }
  return S_OK;
}

HRESULT TextStoreACP::GetStatus(TS_STATUS* pdcs) {
  // No lock is required for this call
  if (!pdcs)
    return E_INVALIDARG;
  pdcs->dwDynamicFlags = TS_SD_UIINTEGRATIONENABLE;
  pdcs->dwStaticFlags = TS_SS_NOHIDDENTEXT;
  return S_OK;
}

HRESULT TextStoreACP::QueryInsert(
  LONG acpTestStart,
  LONG acpTestEnd,
  [[maybe_unused]] ULONG cch,// we allow any length of text, so don't care
  LONG* pacpResultStart,
  LONG* pacpResultEnd) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!pacpResultStart || !pacpResultEnd)
    return E_INVALIDARG;
  // Wrong way around can indicate selection direction
  if (acpTestStart > acpTestEnd)
    std::swap(acpTestStart, acpTestEnd);
  if (acpTestEnd > mOwner->GetTextW().size())
    return E_INVALIDARG;
  *pacpResultStart = acpTestStart;
  *pacpResultEnd = acpTestEnd;
  return S_OK;
}

HRESULT TextStoreACP::GetFormattedText(LONG, LONG, IDataObject** ppDataObject) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!ppDataObject)
    return E_INVALIDARG;
  *ppDataObject = nullptr;
  return E_NOTIMPL;
}

HRESULT TextStoreACP::GetSelection(
  const ULONG ulIndex,
  const ULONG ulCount,
  TS_SELECTION_ACP* pSelection,
  ULONG* pcFetched) {
  if (!HaveReadLock()) {
    return TF_E_NOLOCK;
  }
  if (!pSelection || !pcFetched || ulCount == 0)
    return E_INVALIDARG;
  if (ulIndex != TS_DEFAULT_SELECTION && ulIndex != 0)
    return TS_E_NOSELECTION;
  const auto [selStart, selEnd] = mOwner->GetSelectionW();
  pSelection[0].acpStart = std::min(selStart, selEnd);
  pSelection[0].acpEnd = std::max(selStart, selEnd);
  pSelection[0].style.ase = TS_AE_END;
  pSelection[0].style.fInterimChar = FALSE;
  *pcFetched = 1;
  return S_OK;
}

HRESULT TextStoreACP::SetSelection(ULONG ulCount, const TS_SELECTION_ACP* sel) {
  if (!HaveWriteLock()) {
    return TF_E_NOLOCK;
  }
  if (ulCount != 1 || !sel)
    return E_INVALIDARG;
  const auto guard = mOwner->GetAutomationActivityGuard();
  mOwner->SetSelectionW(sel[0].acpStart, sel[0].acpEnd);
  return S_OK;
}

HRESULT TextStoreACP::GetText(
  LONG acpStart,
  LONG acpEnd,
  WCHAR* pchPlain,
  ULONG cchPlainReq,
  ULONG* pcchPlainOut,
  TS_RUNINFO* prgRunInfo,
  ULONG cRunInfoReq,
  ULONG* pcRunInfoOut,
  LONG* pacpNext) {
  if (!HaveReadLock()) {
    return TF_E_NOLOCK;
  }
  if (!pcchPlainOut)
    return E_INVALIDARG;
  const auto w = mOwner->GetTextW();
  acpEnd = (acpEnd == -1) ? static_cast<LONG>(w.size()) : acpEnd;
  if (acpEnd < acpStart)
    std::swap(acpStart, acpEnd);
  acpStart = std::clamp<LONG>(acpStart, 0, acpEnd);
  const LONG count = std::clamp<LONG>(
    acpEnd - acpStart, 0, static_cast<LONG>(w.size() - acpStart));
  const auto slice = std::wstring_view {w}.substr(acpStart, count);
  if (pchPlain && cchPlainReq) {
    const auto toCopy
      = std::min<ULONG>(cchPlainReq, static_cast<ULONG>(slice.size()));
    memcpy(pchPlain, slice.data(), toCopy * sizeof(WCHAR));
  }
  *pcchPlainOut = static_cast<ULONG>(slice.size());
  if (pacpNext)
    *pacpNext = acpEnd;
  if (cRunInfoReq) {
    if (!(pcRunInfoOut && prgRunInfo)) {
      return E_INVALIDARG;
    }
    prgRunInfo[0] = {
      static_cast<ULONG>(count),
      TS_RT_PLAIN,
    };
    *pcRunInfoOut = 1;// single run
  }

  return S_OK;
}

HRESULT TextStoreACP::SetText(
  [[maybe_unused]] DWORD dwFlags,
  LONG acpStart,
  LONG acpEnd,
  const WCHAR* pchText,
  ULONG cch,
  TS_TEXTCHANGE* pChange) {
  if (!HaveWriteLock()) {
    return TF_E_NOLOCK;
  }
  if (acpStart > acpEnd)
    std::swap(acpStart, acpEnd);

  const auto cur = mOwner->GetTextW();
  const std::wstring_view add {pchText, cch};
  const auto newText
    = std::format(L"{}{}{}", cur.substr(0, acpStart), add, cur.substr(acpEnd));
  const auto caret = acpStart + cch;
  if (pChange) {
    pChange->acpStart = acpStart;
    pChange->acpOldEnd = acpEnd;
    pChange->acpNewEnd = static_cast<LONG>(caret);
  }
  const auto guard = mOwner->GetAutomationActivityGuard();
  mOwner->SetTextW(newText);
  mOwner->SetSelectionW(caret, caret);
  return S_OK;
}

HRESULT TextStoreACP::GetEndACP(LONG* pacp) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!pacp)
    return E_INVALIDARG;
  *pacp = static_cast<LONG>(mOwner->GetTextW().size());
  return S_OK;
}

HRESULT TextStoreACP::GetActiveView(TsViewCookie* pvcView) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!pvcView)
    return E_INVALIDARG;
  *pvcView = 1;
  return S_OK;
}

HRESULT TextStoreACP::GetACPFromPoint(
  TsViewCookie,
  const POINT* pt,
  DWORD,
  LONG* pacp) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!pt || !pacp)
    return E_INVALIDARG;
  // Simplify: return end
  return GetEndACP(pacp);
}

HRESULT TextStoreACP::GetTextExt(
  TsViewCookie,
  LONG acpStart,
  LONG acpEnd,
  RECT* prc,
  BOOL* pfClipped) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!prc || !pfClipped)
    return E_INVALIDARG;
  if (acpStart > acpEnd)
    std::swap(acpStart, acpEnd);

  const auto [widgetRect, clipped]
    = mOwner->GetTextBoundingBoxW(acpStart, acpEnd);
  *pfClipped = clipped;

  const Rect canvasRect {
    widgetRect.GetTopLeft() + mOwner->GetTopLeftCanvasPoint(),
    widgetRect.mSize,
  };

  const auto& window = *mOwner->GetOwnerWindow();
  const auto [left, top]
    = window.CanvasPointToNativePoint(canvasRect.GetTopLeft());
  const auto [right, bottom]
    = window.CanvasPointToNativePoint(canvasRect.GetBottomRight());

  *prc = RECT {
    .left = left,
    .top = top,
    .right = right,
    .bottom = bottom,
  };

  return S_OK;
}

HRESULT TextStoreACP::GetScreenExt(TsViewCookie, RECT* prc) {
  if (!HaveReadLock()) {
    return TS_E_NOLOCK;
  }
  if (!prc)
    return E_INVALIDARG;
  const auto widgetRect = mOwner->GetContentRect();
  const Rect canvasRect {
    widgetRect.GetTopLeft() + mOwner->GetTopLeftCanvasPoint(),
    widgetRect.mSize,
  };

  const auto& window = *mOwner->GetOwnerWindow();
  const auto [left, top]
    = window.CanvasPointToNativePoint(canvasRect.GetTopLeft());
  const auto [right, bottom]
    = window.CanvasPointToNativePoint(canvasRect.GetBottomRight());
  *prc = RECT {left, top, right, bottom};
  return S_OK;
}

HRESULT TextStoreACP::InsertTextAtSelection(
  DWORD,
  const WCHAR* pchText,
  ULONG cch,
  LONG* pacpStart,
  LONG* pacpEnd,
  TS_TEXTCHANGE* pChange) {
  if (!HaveWriteLock()) {
    return TS_E_NOLOCK;
  }
  if (!(pacpStart && pacpEnd && pChange))
    return E_INVALIDARG;
  // Map the current selection and call SetText
  TS_SELECTION_ACP sel {};
  ULONG fetched {};
  GetSelection(0, 1, &sel, &fetched);

  const auto ret = SetText(0, sel.acpStart, sel.acpEnd, pchText, cch, pChange);
  if (SUCCEEDED(ret))
    *pacpStart = *pacpEnd = pChange->acpNewEnd;
  return ret;
}

TSFThreadManager& TSFThreadManager::Get() {
  thread_local TSFThreadManager s;
  return s;
}

void TSFThreadManager::Initialize(HWND) {
  if (mInitialized)
    return;
  mThreadMgr = wil::CoCreateInstanceNoThrow<ITfThreadMgr>(CLSID_TF_ThreadMgr);
  if (!mThreadMgr)
    return;
  CheckHResult(mThreadMgr->Activate(&mClientId));
  mInitialized = true;
}

void TSFThreadManager::Uninitialize() {
  if (!mInitialized)
    return;
  if (mThreadMgr) {
    mThreadMgr->Deactivate();
  }
  mThreadMgr.reset();
  mInitialized = false;
}

TSFThreadManager::Document TSFThreadManager::ActivateFor(HWND, TextBox* owner) {
  Document d {};
  if (!mInitialized || !mThreadMgr)
    return d;
  mThreadMgr->CreateDocumentMgr(&d.mDocMgr);
  if (!d.mDocMgr)
    return d;
  // Create store and context
  d.mStore = wil::com_ptr_nothrow<TextStoreACP>(new TextStoreACP(owner));
  DWORD editCookie {};
  CheckHResult(d.mDocMgr->CreateContext(
    mClientId,
    0,
    static_cast<ITextStoreACP*>(d.mStore.get()),
    &d.mContext,
    &editCookie));

  if (!d.mContext) {
    return {};
  }

  CheckHResult(d.mDocMgr->Push(d.mContext.get()));
  return d;
}

void TSFThreadManager::Deactivate(Document& doc) {
  if (doc.mDocMgr && doc.mContext) {
    doc.mDocMgr->Pop(TF_POPF_ALL);
  }
  doc.mContext.reset();
  doc.mStore.reset();
  doc.mDocMgr.reset();
}

void TSFThreadManager::SetFocus(const HWND hwnd, Document* doc) {
  wil::com_ptr<ITfDocumentMgr> previous;
  CheckHResult(mThreadMgr->SetFocus(doc->mDocMgr.get()));
  CheckHResult(mThreadMgr->AssociateFocus(
    hwnd, doc ? doc->mDocMgr.get() : nullptr, std::out_ptr(previous)));
  auto source = doc->mContext.try_query<ITfSource>();
  DWORD cookie {};
  source->AdviseSink(
    IID_ITextStoreACP, static_cast<ITextStoreACP*>(doc->mStore.get()), &cookie);
}

}// namespace FredEmmott::GUI::win32_detail
