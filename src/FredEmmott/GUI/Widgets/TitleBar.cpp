// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TitleBar.hpp"

#include "Button.hpp"
#include "FredEmmott/GUI/IconProvider.hpp"
#include "FredEmmott/GUI/StaticTheme/TitleBar.hpp"
#include "FredEmmott/utility/almost_equal.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass ContentContainerStyleClass {"TitleBar/Content"};
constexpr LiteralStyleClass ChromeButtonsContainerStyleClass {
  "TitleBar/ChromeButtons"};
constexpr std::string_view MinimizeGlyph = "\ue921";
constexpr std::string_view MaximizeGlyph = "\ue922";
constexpr std::string_view RestoreGlyph = "\ue923";
constexpr std::string_view CloseGlyph = "\ue8bb";

class TitleBarIconButton : public Button {
 public:
  using Button::Button;

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  ApplicationIconProvider mProvider;
  mutable std::unique_ptr<ImportedTexture> mTexture;
  mutable uint64_t mPreviousPixelHeight {};
  mutable Rect mSourceRect {};
};

void TitleBarIconButton::PaintOwnContent(
  Renderer* renderer,
  const Rect& destRect,
  const Style&) const {
  FUI_ASSERT(destRect.GetHeight() > 0);
  const auto pixelHeight = renderer->GetPhysicalLength(destRect.GetHeight());
  FUI_ASSERT(pixelHeight > 0);

  if (pixelHeight != mPreviousPixelHeight) {
    mTexture.reset();
  }
  if (!mTexture) {
    const auto bitmap = mProvider.GetBestBitmap(pixelHeight);
    mTexture = renderer->ImportBitmap(bitmap);
    mPreviousPixelHeight = pixelHeight;
    mSourceRect = {
      Point {},
      Size {
        static_cast<float>(bitmap.mWidth),
        static_cast<float>(bitmap.mHeight),
      },
    };
  }
  FUI_ASSERT(mTexture);
  renderer->DrawTexture(mSourceRect, destRect, mTexture.get(), nullptr, 0);
}

}// namespace
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

bool TitleBar::ConsumeWasIconActivated() {
  return mIconButton->ConsumeWasActivated();
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

  const auto iconTopLeft = mIconButton->GetTopLeftCanvasPoint(this);
  const auto iconYoga = mIconButton->GetLayoutNode();
  const auto iconWidth = YGNodeLayoutGetWidth(iconYoga);
  const auto iconHeight = YGNodeLayoutGetHeight(iconYoga);

  const auto minTopLeft = mMinimizeButton->GetTopLeftCanvasPoint(this);
  const auto maxTopLeft = mMaximizeButton->GetTopLeftCanvasPoint(this);
  const auto closeTopLeft = mCloseButton->GetTopLeftCanvasPoint(this);

  FUI_ASSERT(utility::almost_equal(maxTopLeft.mX, minTopLeft.mX + height));

  return Rects {
    .mFullArea = {topLeft, Size {width, height}},
    .mIconButton = {iconTopLeft, Size {iconWidth, iconHeight}},
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

  mIconButton = new TitleBarIconButton(0, TitleBarIconStyle(), {});
  mTitleLabel = new Label(0, TitleBarTitleStyle());
  mSubtitleLabel = new Label(0, TitleBarSubtitleStyle());
  content->SetChildren({mIconButton, mTitleLabel, mSubtitleLabel});

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