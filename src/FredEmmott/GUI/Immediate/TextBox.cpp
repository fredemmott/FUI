// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

namespace FredEmmott::GUI::Immediate {

[[nodiscard]]
TextBoxResult TextBox(std::string* text, const ID id) {
  if (!text) {
    throw std::logic_error("TextBox requires a non-null text pointer");
  }

  const auto w = immediate_detail::ChildlessWidget<Widgets::TextBox>(id);
  if (std::exchange(w->mChanged, false)) {
    *text = w->GetText();
    return {w, true};
  }
  w->SetText(*text, Widgets::TextBox::ChangeBehavior::DoNotMarkChanged);
  return {w, false};
}

}// namespace FredEmmott::GUI::Immediate