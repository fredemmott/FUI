// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TSFTextStore.hpp"

#include <wil/com.h>
#include <wil/resource.h>

#include <FredEmmott/GUI/Widgets/TextBox.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <algorithm>

namespace FredEmmott::GUI::win32_detail {

using namespace Widgets;

namespace {
struct WideCharDataObject : COMImplementation<IDataObject> {
  WideCharDataObject() {
    CheckHResult(CreateDataAdviseHolder(std::out_ptr(mAdviseHolder)));
  }
  explicit WideCharDataObject(const std::wstring_view text)
    : WideCharDataObject() {
    mText = text;
  }

  HRESULT GetData(FORMATETC* format, STGMEDIUM* out) override {
    if (!SUCCEEDED(QueryGetData(format))) {
      return DV_E_FORMATETC;
    }
    const auto byteCount = mText.size() * sizeof(decltype(mText)::value_type);
    auto buffer = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if (!buffer) {
      return E_OUTOFMEMORY;
    }
    memcpy(GlobalLock(buffer), mText.data(), byteCount);
    GlobalUnlock(buffer);
    *out = {
      .tymed = TYMED_HGLOBAL,
      .hGlobal = buffer,
      .pUnkForRelease = nullptr,
    };
    return S_OK;
  }

  HRESULT QueryGetData(FORMATETC* format) override {
    if (format->cfFormat != CF_UNICODETEXT) {
      return DV_E_FORMATETC;
    }
    if (!(format->tymed & TYMED_HGLOBAL)) {
      return DV_E_FORMATETC;
    }
    return S_OK;
  }

  void SetText(const std::wstring_view data) {
    mText = data;
    CheckHResult(mAdviseHolder->SendOnDataChange(this, 0, 0));
  }

  HRESULT SetData(FORMATETC*, STGMEDIUM*, BOOL) override {
    return E_NOTIMPL;
  }

  HRESULT GetDataHere(FORMATETC*, STGMEDIUM*) override {
    return E_NOTIMPL;
  }
  HRESULT GetCanonicalFormatEtc(FORMATETC*, FORMATETC*) override {
    return E_NOTIMPL;
  }
  HRESULT EnumFormatEtc(DWORD, IEnumFORMATETC**) override {
    return E_NOTIMPL;
  }
  HRESULT DAdvise(
    FORMATETC* fetc,
    DWORD advf,
    IAdviseSink* sink,
    DWORD* connection) override {
    return mAdviseHolder->Advise(this, fetc, advf, sink, connection);
  }
  HRESULT DUnadvise(DWORD connection) override {
    return mAdviseHolder->Unadvise(connection);
  }
  HRESULT EnumDAdvise(IEnumSTATDATA** data) override {
    return mAdviseHolder->EnumAdvise(data);
  }

 private:
  std::wstring mText;
  wil::com_ptr<IDataAdviseHolder> mAdviseHolder;
};
}// namespace

// -------- TextStoreACP ---------

TextStoreACP::TextStoreACP(TextBox* owner) : mOwner(owner) {}

HRESULT TextStoreACP::AdviseSink(REFIID riid, IUnknown* punk, DWORD dwMask) {
  if (riid != __uuidof(ITextStoreACPSink)) {
    return E_INVALIDARG;
  }
  if (mSink) {
    __debugbreak();
  }
  mSink = wil::com_query<ITextStoreACPSink>(punk);
  if (!mSink)
    return E_NOINTERFACE;
  mSinkMask = dwMask;

  if (dwMask & TS_AS_LAYOUT_CHANGE)
    CheckHResult(mSink->OnLayoutChange(TS_LC_CREATE, 1));

  return S_OK;
}

HRESULT TextStoreACP::UnadviseSink(IUnknown* punk) {
  if (wil::com_query<ITextStoreACPSink>(punk) != mSink) {
    // TODO: ... runtime_error?
    __debugbreak();
  }
  mSink.reset();
  mSinkMask = 0;
  return S_OK;
}

HRESULT TextStoreACP::RequestLock(DWORD dwLockFlags, HRESULT* phrSession) {
  if (!phrSession)
    return E_INVALIDARG;
  if (mLocked) {
    __debugbreak();
    *phrSession = TS_E_SYNCHRONOUS;
    return S_OK;
  }

  mLocked = true;
  const auto unlock = wil::scope_exit([this] { mLocked = false; });
  if (mSink) {
    *phrSession = mSink->OnLockGranted(dwLockFlags);
    CheckHResult(*phrSession);
  } else {
    *phrSession = S_OK;
  }
  return S_OK;
}

HRESULT TextStoreACP::GetStatus(TS_STATUS* pdcs) {
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
  if (!pacpResultStart || !pacpResultEnd)
    return E_INVALIDARG;
  // Wrong way around can indicate selection direction
  if (acpTestStart > acpTestEnd)
    std::swap(acpTestStart, acpTestEnd);
  if (acpTestEnd > GetWideText().size())
    return E_INVALIDARG;
  *pacpResultStart = acpTestStart;
  *pacpResultEnd = acpTestEnd;
  return S_OK;
}

std::wstring TextStoreACP::GetWideText() const {
  return Utf8ToWide(mOwner->GetText());
}

HRESULT TextStoreACP::GetFormattedText(LONG, LONG, IDataObject** ppDataObject) {
  if (!ppDataObject)
    return E_INVALIDARG;
  *ppDataObject = nullptr;
  return E_NOTIMPL;
}
HRESULT TextStoreACP::OnStartComposition(ITfCompositionView*, BOOL* pfOk) {
  if (!pfOk)
    return E_INVALIDARG;
  *pfOk = TRUE;
  return S_OK;
}

HRESULT TextStoreACP::OnUpdateComposition(ITfCompositionView*, ITfRange*) {
  return S_OK;
}

HRESULT TextStoreACP::OnEndComposition(ITfCompositionView*) {
  return S_OK;
}

static std::size_t Utf16ToUtf8Index(
  const std::string_view u8,
  size_t u16Index) {
  if (u16Index == 0)
    return 0;
  const auto w = Utf8ToWide(u8);
  const auto clamped = std::min<std::size_t>(u16Index, w.size());
  std::wstring_view prefix {w.c_str(), clamped};
  return WideToUtf8(std::wstring(prefix)).size();
}

static LONG Utf8ToUtf16Index(const std::string& u8, size_t u8Index) {
  const auto w = Utf8ToWide(u8.substr(0, std::min(u8Index, u8.size())));
  return static_cast<LONG>(w.size());
}

HRESULT TextStoreACP::GetSelection(
  const ULONG ulIndex,
  const ULONG ulCount,
  TS_SELECTION_ACP* pSelection,
  ULONG* pcFetched) {
  if (!pSelection || !pcFetched || ulCount == 0)
    return E_INVALIDARG;
  if (ulIndex != TS_DEFAULT_SELECTION && ulIndex != 0)
    return TS_E_NOSELECTION;
  const auto [selStart, selEnd] = mOwner->GetSelection();
  const auto& text = mOwner->GetText();
  pSelection[0].acpStart
    = Utf8ToUtf16Index(std::string {text}, std::min(selStart, selEnd));
  pSelection[0].acpEnd
    = Utf8ToUtf16Index(std::string {text}, std::max(selStart, selEnd));
  pSelection[0].style.ase = TS_AE_END;
  pSelection[0].style.fInterimChar = FALSE;
  *pcFetched = 1;
  return S_OK;
}

HRESULT TextStoreACP::SetSelection(ULONG ulCount, const TS_SELECTION_ACP* sel) {
  if (ulCount != 1 || !sel)
    return E_INVALIDARG;
  const auto& text = mOwner->GetText();
  const auto start = Utf16ToUtf8Index(text, sel[0].acpStart);
  const auto end = Utf16ToUtf8Index(text, sel[0].acpEnd);
  mOwner->SetSelection(start, end);
  if (mSink && (mSinkMask & TS_AS_SEL_CHANGE)) {
    mSink->OnSelectionChange();
  }
  return S_OK;
}

HRESULT TextStoreACP::GetText(
  LONG acpStart,
  LONG acpEnd,
  WCHAR* pchPlain,
  ULONG cchPlainReq,
  ULONG* pcchPlainOut,
  TS_RUNINFO*,
  ULONG /*cRunInfoReq*/,
  ULONG* pcRunInfoOut,
  LONG* pacpNext) {
  if (!pcchPlainOut)
    return E_INVALIDARG;
  const auto w = GetWideText();
  if (acpEnd < acpStart)
    std::swap(acpStart, acpEnd);
  const LONG end = (acpEnd == -1) ? static_cast<LONG>(w.size()) : acpEnd;
  const LONG start = std::clamp<LONG>(acpStart, 0, end);
  const LONG count
    = std::clamp<LONG>(end - start, 0, static_cast<LONG>(w.size() - start));
  const auto slice = std::wstring_view {w}.substr(start, count);
  if (pchPlain && cchPlainReq) {
    const auto toCopy
      = std::min<ULONG>(cchPlainReq, static_cast<ULONG>(slice.size()));
    memcpy(pchPlain, slice.data(), toCopy * sizeof(WCHAR));
  }
  *pcchPlainOut = static_cast<ULONG>(slice.size());
  if (pcRunInfoOut)
    *pcRunInfoOut = 0;// single run
  if (pacpNext)
    *pacpNext = end;
  return S_OK;
}

HRESULT TextStoreACP::SetText(
  DWORD,
  LONG acpStart,
  LONG acpEnd,
  const WCHAR* pchText,
  ULONG cch,
  TS_TEXTCHANGE* pChange) {
  if (acpStart > acpEnd)
    std::swap(acpStart, acpEnd);

  const auto cur = std::string {mOwner->GetText()};
  const auto left = Utf16ToUtf8Index(cur, acpStart);
  const auto right = Utf16ToUtf8Index(cur, acpEnd);
  const std::wstring_view add {pchText, cch};
  const auto addU8 = WideToUtf8(std::wstring(add));
  const auto newText
    = std::format("{}{}{}", cur.substr(0, left), addU8, cur.substr(right));
  mOwner->SetText(newText);
  const auto caret = left + addU8.size();
  mOwner->SetSelection(caret, caret);
  if (pChange) {
    pChange->acpStart = static_cast<LONG>(acpStart);
    pChange->acpOldEnd = static_cast<LONG>(acpEnd);
    pChange->acpNewEnd = static_cast<LONG>(acpStart + cch);
  }
  return S_OK;
}

HRESULT TextStoreACP::GetEndACP(LONG* pacp) {
  if (!pacp)
    return E_INVALIDARG;
  const auto w = GetWideText();
  *pacp = static_cast<LONG>(w.size());
  return S_OK;
}

HRESULT TextStoreACP::GetActiveView(TsViewCookie* pvcView) {
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
  __debugbreak();
  if (!prc || !pfClipped)
    return E_INVALIDARG;
  if (acpStart > acpEnd)
    std::swap(acpStart, acpEnd);

  const auto [widgetRect, clipped] = mOwner->GetTextBoundingBox(
    Utf16ToUtf8Index(mOwner->GetText(), acpStart),
    Utf16ToUtf8Index(mOwner->GetText(), acpEnd));
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
  __debugbreak();
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
  __debugbreak();
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

// -------- TSFThreadManager ---------

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

bool TSFThreadManager::WndProc(HWND, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (!mThreadMgr) {
    return false;
  }
  if (!mKeystrokeMgr) {
    mKeystrokeMgr = wil::try_com_query<ITfKeystrokeMgr>(mThreadMgr.get());
  }
  if (!mKeystrokeMgr) {
    return false;
  }

  const auto km = mKeystrokeMgr.get();
  BOOL eaten {FALSE};

  switch (msg) {
    case WM_KEYDOWN:
      CheckHResult(km->TestKeyDown(wParam, lParam, &eaten));
      if (!eaten) {
        return false;
      }
      CheckHResult(km->KeyDown(wParam, lParam, &eaten));
      return eaten;
    case WM_KEYUP:
      CheckHResult(km->TestKeyUp(wParam, lParam, &eaten));
      if (!eaten) {
        return false;
      }
      CheckHResult(km->KeyUp(wParam, lParam, &eaten));
      return eaten;
    case WM_CHAR:
      // TODO: fallback if not active
      return true;
    default:
      return false;
  }
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
    static_cast<ITextStoreACP2*>(d.mStore.get()),
    &d.mContext,
    &editCookie));
  if (!d.mContext) {
    __debugbreak();
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
  CheckHResult(mThreadMgr->AssociateFocus(
    hwnd, doc ? doc->mDocMgr.get() : nullptr, std::out_ptr(previous)));
  auto source = doc->mContext.try_query<ITfSource>();
  DWORD cookie {};
  source->AdviseSink(IID_ITextStoreACP2, doc->mStore.get(), &cookie);
}

}// namespace FredEmmott::GUI::win32_detail
