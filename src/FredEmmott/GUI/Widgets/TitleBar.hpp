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
    Rect mMinimizeButton;
    Rect mMaximizeButton;
    Rect mCloseButton;
  };
  enum class ChromeButton {
    Minimize,
    Maximize,
    Close,
  };
  explicit TitleBar(id_type id);
  ~TitleBar() override;

  void SetTitle(std::string_view);
  void SetSubtitle(std::string_view);

  [[nodiscard]]
  std::optional<ChromeButton> GetHoveredButton() const noexcept;
  [[nodiscard]]
  std::optional<ChromeButton> GetPressedButton() const noexcept;

  [[nodiscard]]
  bool ConsumeWasMinimizeActivated();
  [[nodiscard]]
  bool ConsumeWasMaximizeActivated();
  [[nodiscard]]
  bool ConsumeWasCloseActivated();

  void SetIsMaximized(bool);

  Rects GetRects() const;

 private:
  Button* mMinimizeButton {};
  Button* mMaximizeButton {};
  Button* mCloseButton {};

  Label* mMaximizeLabel {};

  Label* mTitleLabel {};
  Label* mSubtitleLabel {};
};

}// namespace FredEmmott::GUI::Widgets