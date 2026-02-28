// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TitleBar.hpp"

#include "Button.hpp"
#include "FredEmmott/GUI/StaticTheme/TitleBar.hpp"
#include "FredEmmott/utility/almost_equal.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

static constexpr LiteralStyleClass ContentContainerStyleClass {
  "TitleBar/Content"};
static constexpr LiteralStyleClass ChromeButtonsContainerStyleClass {
  "TitleBar/ChromeButtons"};
static constexpr std::string_view MinimizeGlyph = "\ue921";
static constexpr std::string_view MaximizeGlyph = "\ue922";
static constexpr std::string_view RestoreGlyph = "\ue923";
static constexpr std::string_view CloseGlyph = "\ue8bb";

TitleBar::~TitleBar() = default;

void TitleBar::SetTitle(const std::string_view text) {
  mTitleLabel->SetText(text);
}
void TitleBar::SetSubtitle(const std::string_view text) {
  mSubtitleLabel->SetText(text);
}

std::optional<TitleBar::ChromeButton> TitleBar::GetHoveredButton()
  const noexcept {
  using enum ChromeButton;
  if (mMinimizeButton->IsHovered()) {
    return Minimize;
  }
  if (mMaximizeButton->IsHovered()) {
    return Maximize;
  }
  if (mCloseButton->IsHovered()) {
    return Close;
  }
  return {};
}

std::optional<TitleBar::ChromeButton> TitleBar::GetPressedButton()
  const noexcept {
  using enum ChromeButton;
  if (mMinimizeButton->IsActive()) {
    return Minimize;
  }
  if (mMaximizeButton->IsActive()) {
    return Maximize;
  }
  if (mCloseButton->IsActive()) {
    return Close;
  }
  return {};
}

bool TitleBar::ConsumeWasMinimizeActivated() {
  return mMinimizeButton->ConsumeWasActivated();
}
bool TitleBar::ConsumeWasMaximizeActivated() {
  return mMaximizeButton->ConsumeWasActivated();
}

bool TitleBar::ConsumeWasCloseActivated() {
  return mCloseButton->ConsumeWasActivated();
}

void TitleBar::SetIsMaximized(const bool isMaximized) {
  if (isMaximized) {
    mMaximizeLabel->SetText(RestoreGlyph);
  } else {
    mMaximizeLabel->SetText(MaximizeGlyph);
  }
}
TitleBar::Rects TitleBar::GetRects() const {
  const auto yoga = this->GetLayoutNode();
  const auto topLeft = this->GetTopLeftCanvasPoint(this);
  const auto width = YGNodeLayoutGetWidth(yoga);
  const auto height = YGNodeLayoutGetHeight(yoga);

  const auto minTopLeft = mMinimizeButton->GetTopLeftCanvasPoint(this);
  const auto maxTopLeft = mMaximizeButton->GetTopLeftCanvasPoint(this);
  const auto closeTopLeft = mCloseButton->GetTopLeftCanvasPoint(this);

  FUI_ASSERT(utility::almost_equal(maxTopLeft.mX, minTopLeft.mX + height));

  return Rects {
    .mFullArea = {topLeft, Size {width, height}},
    .mMinimizeButton = {minTopLeft, Size {height, height}},
    .mMaximizeButton = {maxTopLeft, Size {height, height}},
    .mCloseButton = {closeTopLeft, Size {height, height}},
  };
}

TitleBar::TitleBar(const id_type id)
  : Widget(
      id,
      LiteralStyleClass("TitleBar"),
      StaticTheme::TitleBar::DefaultTitleBarStyle()) {
  using namespace StaticTheme::TitleBar;
  auto content = new Widget(
    0, ContentContainerStyleClass, TitleBarContentContainerStyle());
  auto chromeButtons = new Widget(0, ChromeButtonsContainerStyleClass, {});
  this->SetChildren({content, chromeButtons});

  mTitleLabel = new Label(0, TitleBarTitleStyle());
  mSubtitleLabel = new Label(0, TitleBarSubtitleStyle());
  content->SetChildren({mTitleLabel, mSubtitleLabel});

  chromeButtons->SetChildren({
    mMinimizeButton = new Button(0, WindowMinimizeMaximizeButtonStyle(), {}),
    mMaximizeButton = new Button(1, WindowMinimizeMaximizeButtonStyle(), {}),
    mCloseButton = new Button(2, WindowCloseButtonStyle(), {}),
  });
  mMinimizeButton->SetChildren({
    (new Label(0, ImmutableStyle {}))->SetText(MinimizeGlyph),
  });
  mMaximizeButton->SetChildren({
    mMaximizeLabel = new Label(0, ImmutableStyle {}),
  });
  mCloseButton->SetChildren(
    {(new Label(0, ImmutableStyle {}))->SetText(CloseGlyph)});
  mMaximizeLabel->SetText(MaximizeGlyph);
}

}// namespace FredEmmott::GUI::Widgets