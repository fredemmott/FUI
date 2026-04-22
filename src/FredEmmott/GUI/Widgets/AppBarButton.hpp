// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Button.hpp"

namespace FredEmmott::GUI::Widgets {
class Label;

class AppBarButton final : public Button {
 public:
  enum class LabelPosition {
    Bottom,
    Right,
    Collapsed,
  };

  explicit AppBarButton(Window*);
  ~AppBarButton() override;

  void SetLabel(std::string_view);
  void SetGlyph(std::string_view);
  void SetIsCompact(bool);
  void SetLabelPosition(LabelPosition);
  void SetIsOutline(bool);

  [[nodiscard]]
  const Widget* GetFocusDelegate() const noexcept override {
    return mBorderContainer;
  }

 private:
  LabelPosition mLabelPosition {LabelPosition::Bottom};
  bool mIsCompact {false};
  bool mIsOutline {false};

  /* To reproduce WinUI3 accurately, we annoyingly have 3 layers of containers:
   * - this widget (hit-test target for clicks)
   *   - border container: actually has the border, but has its own margins
   *     within this container
   *     - content container
   *       - glyph
   *       - label
   *       - chevron
   */
  Widget* mBorderContainer {};
  Widget* mContentContainer {};
  Label* mGlyph {};
  Label* mLabel {};
  // Not yet supported
  // Label* mChevron {};
};

}// namespace FredEmmott::GUI::Widgets