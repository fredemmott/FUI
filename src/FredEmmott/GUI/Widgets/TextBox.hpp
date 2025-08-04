// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>
#include <wil/com.h>
#include <wil/winrt.h>
#include <winrt/windows.ui.text.core.h>

#include "Focusable.hpp"
#include "FredEmmott/GUI/detail/win32_detail/WinRTEventHandler.hpp"
#include "Label.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI {
class Window;
}
namespace FredEmmott::GUI::Widgets {

class TextBox : public Widget, public IFocusable {
 public:
  TextBox(std::size_t id);
  ~TextBox() override;

  void SetText(std::string_view);

 protected:
  void Tick() override;

 private:
  using unique_event_token = wil::unique_winrt_event_token<
    ABI::Windows::UI::Text::Core::ICoreTextEditContext>;
  Window* mWindow {};
  std::string mText;
  std::size_t mSelectionStart {};
  std::size_t mSelectionEnd {};
  bool mIsFocused {false};

  Label* mBeforeSelection {};
  Widget* mCaret {};
  Label* mSelection {};
  Label* mAfterSelection {};

  wil::com_ptr<ABI::Windows::UI::Text::Core::ICoreTextEditContext>
    mTextEditContext;

  unique_event_token mTextRequestedToken;
  unique_event_token mLayoutRequestedToken;
  unique_event_token mTextUpdatingToken;
  unique_event_token mSelectionRequestedToken;
  unique_event_token mSelectionUpdatingToken;

  HRESULT OnTextRequested(
    ABI::Windows::UI::Text::Core::ICoreTextEditContext*,
    ABI::Windows::UI::Text::Core::ICoreTextTextRequestedEventArgs*);
  HRESULT OnTextUpdating(
    ABI::Windows::UI::Text::Core::ICoreTextEditContext*,
    ABI::Windows::UI::Text::Core::ICoreTextTextUpdatingEventArgs*);
  HRESULT OnLayoutRequested(
    ABI::Windows::UI::Text::Core::ICoreTextEditContext*,
    ABI::Windows::UI::Text::Core::ICoreTextLayoutRequestedEventArgs*);
  HRESULT OnSelectionRequested(
    ABI::Windows::UI::Text::Core::ICoreTextEditContext*,
    ABI::Windows::UI::Text::Core::ICoreTextSelectionRequestedEventArgs*);
  HRESULT OnSelectionUpdating(
    ABI::Windows::UI::Text::Core::ICoreTextEditContext*,
    ABI::Windows::UI::Text::Core::ICoreTextSelectionUpdatingEventArgs*);
};

}// namespace FredEmmott::GUI::Widgets