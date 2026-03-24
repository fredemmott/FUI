// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button;
class Label;

class TitleBar final : public Widget {
 public:
  struct Rects {
    Rect mFullArea;
    Rect mIconButton;
    Rect mMinimizeButton;
    Rect mMaximizeButton;
    Rect mCloseButton;
  };
  enum class ChromeButton {
    Minimize,
    Maximize,
    Close,
  };
  TitleBar();
  ~TitleBar() override;

  void SetTitle(std::string_view);
  void SetSubtitle(std::string_view);

  [[nodiscard]]
  std::optional<ChromeButton> GetHoveredButton() const noexcept;
  [[nodiscard]]
  std::optional<ChromeButton> GetPressedButton() const noexcept;

  [[nodiscard]]
  bool ConsumeWasIconActivated();
  [[nodiscard]]
  bool ConsumeWasMinimizeActivated();
  [[nodiscard]]
  bool ConsumeWasMaximizeActivated();
  [[nodiscard]]
  bool ConsumeWasCloseActivated();

  void SetIsMaximized(bool);
  void SetIsActiveWindow(bool);

  [[nodiscard]]
  Rects GetRects() const;

  void SetLeftWidgets(const std::vector<Widget*>&);

 private:
  Widget* mContent {};

  // On Win32, clicking this should trigger the restore/min/max dropdown
  Button* mIconButton {};

  Button* mMinimizeButton {};
  Button* mMaximizeButton {};
  Button* mCloseButton {};

  Label* mMaximizeLabel {};

  Label* mTitleLabel {};
  Label* mSubtitleLabel {};
  bool mWindowIsActive {true};
};

}// namespace FredEmmott::GUI::Widgets