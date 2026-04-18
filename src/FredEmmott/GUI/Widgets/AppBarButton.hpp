// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Button.hpp"

namespace FredEmmott::GUI::Widgets {
class Label;

class AppBarButton final : public Button {
 public:
  explicit AppBarButton(Window*);
  ~AppBarButton() override;

 private:
  /* To reproduce WinUI3 accurately, we annoyingly have 3 layers of containers:
   * - this widget (hit-test target for clicks)
   *   - border container: actually has the border, but has its' own margins
   *     within this container
   *     - content container: self-descriptive
   *       - glyph
   *       - label
   *       - chevron
   */
  Widget* mBorderContainer {};
  Widget* mContentContainer {};
  Label* mGlyph {};
  Label* mLabel {};
  Label* mChevron {};
};

}// namespace FredEmmott::GUI::Widgets