// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

#include "FredEmmott/GUI/FocusManager.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"

using namespace ABI::Windows::UI::Text::Core;

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace win32_detail;

ABI::Windows::Foundation::Rect GetBoundingRect(Widget* widget, Window* window) {
  const auto yoga = widget->GetLayoutNode();

  const auto canvasTL = widget->GetTopLeftCanvasPoint();
  const auto canvasBR = canvasTL
    + Point {
      YGNodeLayoutGetWidth(yoga),
      YGNodeLayoutGetHeight(yoga),
    };
  const auto nativeTL = window->CanvasPointToNativePoint(canvasTL).as<Point>();
  const auto nativeBR = window->CanvasPointToNativePoint(canvasBR).as<Point>();
  return {
    nativeTL.mX,
    nativeTL.mY,
    nativeBR.mX - nativeTL.mX,
    nativeBR.mY - nativeTL.mY,
  };
}

auto& TextBoxStyles() {
  static const ImmutableStyle ret {
    Style().FlexDirection(YGFlexDirectionRow),
  };
  return ret;
}

auto GetTextServicesManager() {
  thread_local ICoreTextServicesManager* sTsm {};
  if (!sTsm) {
    const auto statics
      = wil::GetActivationFactory<ICoreTextServicesManagerStatics>(
        RuntimeClass_Windows_UI_Text_Core_CoreTextServicesManager);

    wil::com_ptr<ICoreTextServicesManager> tsm;
    CheckHResult(statics->GetForCurrentView(&tsm));
    sTsm = tsm.get();
  }
  return sTsm;
}

}// namespace

TextBox::TextBox(const std::size_t id) : Widget(id, TextBoxStyles(), {}) {
  mWindow = Immediate::immediate_detail::tWindow;

  static const ImmutableStyle CaretStyle {
    Style().Width(1).BackgroundColor(Colors::Red),
  };
  static const ImmutableStyle SelectionStyle {
    Style().BackgroundColor(Colors::Blue).Color(Colors::White)};

  this->SetChildren({
    mBeforeSelection = new Label(0),
    mCaret = new Widget(0, CaretStyle),
    mSelection = new Label(0),
    mAfterSelection = new Label(0),
  });

  const auto tsm = GetTextServicesManager();
  CheckHResult(tsm->CreateEditContext(std::out_ptr(mTextEditContext)));
  CheckHResult(mTextEditContext->put_InputScope(CoreTextInputScope_Text));
#define CONNECT_EVENT_HANDLER(NAME) \
  m##NAME##Token = WI_MakeUniqueWinRtEventToken( \
    NAME, \
    mTextEditContext.get(), \
    (new WinRTEventHandler< \
      CoreTextEditContext, \
      ICoreTextEditContext, \
      CoreText##NAME##EventArgs, \
      ICoreText##NAME##EventArgs>( \
      std::bind_front(&TextBox::On##NAME, this))));

  CONNECT_EVENT_HANDLER(TextRequested)
  CONNECT_EVENT_HANDLER(TextUpdating)
  CONNECT_EVENT_HANDLER(LayoutRequested)
  CONNECT_EVENT_HANDLER(SelectionRequested)
  CONNECT_EVENT_HANDLER(SelectionUpdating)
#undef CONNECT_EVENT_HANDLER

  this->SetText("Hello, world");
}

TextBox::~TextBox() = default;

void TextBox::SetText(const std::string_view text) {
  if (text == mText) {
    return;
  }

  mText = std::string {text};
  mSelectionStart = 0;
  mSelectionEnd = 0;
  mBeforeSelection->SetText({});
  mSelection->SetText({});
  mAfterSelection->SetText(mText);
  CheckHResult(mTextEditContext->NotifyTextChanged(
    {0, static_cast<int32_t>(mText.size())}, mText.size(), {}));
}

void TextBox::Tick() {
  const auto isFocused = FocusManager::Get()->IsWidgetFocused(this);
  if (isFocused == mIsFocused) {
    return;
  }
  mIsFocused = isFocused;
  if (isFocused) {
    CheckHResult(mTextEditContext->NotifyFocusEnter());
  } else {
    CheckHResult(mTextEditContext->NotifyFocusLeave());
  }
}

HRESULT TextBox::OnTextRequested(
  ICoreTextEditContext*,
  ICoreTextTextRequestedEventArgs* args) {
  wil::com_ptr<ICoreTextTextRequest> request;
  CheckHResult(args->get_Request(std::out_ptr(request)));

  CoreTextRange range {};
  CheckHResult(request->get_Range(&range));

  if (
    range.StartCaretPosition > range.EndCaretPosition
    || range.StartCaretPosition < 0) {
    return E_INVALIDARG;
  }

  const std::string_view slice {
    mText.data() + range.StartCaretPosition,
    std::min<std::size_t>(range.EndCaretPosition, mText.size())
      - range.StartCaretPosition,
  };
  const auto wide = Utf8ToWide(slice);

  HSTRING_HEADER hstringHeader {};
  HSTRING hstring {};
  CheckHResult(WindowsCreateStringReference(
    wide.data(), wide.size(), &hstringHeader, &hstring));
  CheckHResult(request->put_Text(hstring));

  return S_OK;
}

HRESULT TextBox::OnTextUpdating(
  ICoreTextEditContext*,
  ICoreTextTextUpdatingEventArgs*) {
  __debugbreak();
  return S_OK;
}

HRESULT TextBox::OnLayoutRequested(
  ICoreTextEditContext*,
  ICoreTextLayoutRequestedEventArgs* args) {
  wil::com_ptr<ICoreTextLayoutRequest> request;
  CheckHResult(args->get_Request(std::out_ptr(request)));
  wil::com_ptr<ICoreTextLayoutBounds> layoutBounds;
  CheckHResult(request->get_LayoutBounds(std::out_ptr(layoutBounds)));

  CheckHResult(layoutBounds->put_ControlBounds(GetBoundingRect(this, mWindow)));
  CheckHResult(
    layoutBounds->put_TextBounds(GetBoundingRect(mSelection, mWindow)));

  return S_OK;
}

HRESULT TextBox::OnSelectionRequested(
  ICoreTextEditContext*,
  ICoreTextSelectionRequestedEventArgs* args) {
  wil::com_ptr<ICoreTextSelectionRequest> request;
  CheckHResult(args->get_Request(std::out_ptr(request)));
  request->put_Selection(
    CoreTextRange {
      static_cast<int32_t>(mSelectionStart),
      static_cast<int32_t>(mSelectionEnd),
    });
  return S_OK;
}

HRESULT TextBox::OnSelectionUpdating(
  ICoreTextEditContext*,
  ICoreTextSelectionUpdatingEventArgs*) {
  return S_OK;
}

}// namespace FredEmmott::GUI::Widgets
