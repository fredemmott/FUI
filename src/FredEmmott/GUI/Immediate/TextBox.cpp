// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

#include "FredEmmott/GUI/StaticTheme/TextBox.hpp"
#include "FredEmmott/GUI/Widgets/Button.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

[[nodiscard]]
TextBoxResult TextBox(std::string* text, const ID id) {
  using namespace StaticTheme::TextBox;

  if (!text) [[unlikely]] {
    throw std::logic_error("TextBox requires a non-null text pointer");
  }

  const auto w = immediate_detail::BeginWidget<Widgets::TextBox>(id);
  const auto endWidget = felly::scope_exit([] {
    immediate_detail::EndWidget<Widgets::TextBox>();
  });

  if (!w->GetText().empty()) {
    const auto clearButton = immediate_detail::BeginWidget<Widgets::Button>(
      ID {0},
      DefaultTextBoxButtonStyle(),
      StyleClasses {TextBoxButtonInvisibleWhenInactiveStyleClass});
    Label("\ue894");
    immediate_detail::EndWidget<Widgets::Button>();

    if (clearButton->ConsumeWasActivated()) {
      *text = {};
      if (const auto fm = FocusManager::Get()) {
        fm->GivePointerFocus(w);
      }
    }
  }

  if (w->ConsumeWasChanged()) {
    *text = w->GetText();
    return {w, true};
  }

  w->SetText(*text);
  std::ignore = w->ConsumeWasChanged();

  return {w, false};
}

}// namespace FredEmmott::GUI::Immediate