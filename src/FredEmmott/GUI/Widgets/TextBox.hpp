// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>

#include "Focusable.hpp"
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
  Window* mWindow {};
  std::string mText;
  std::size_t mSelectionStart {};
  std::size_t mSelectionEnd {};
  bool mIsFocused {false};

  Label* mBeforeSelection {};
  Widget* mCaret {};
  Label* mSelection {};
  Label* mAfterSelection {};
};

}// namespace FredEmmott::GUI::Widgets