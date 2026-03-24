// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TitleBar.hpp"

#include <Yoga.h>

#include <felly/numeric_cast.hpp>

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
constexpr LiteralStyleClass TitleBarTitleStyleClass {"TitleBar/Title"};
constexpr LiteralStyleClass TitleBarSubtitleStyleClass {"TitleBar/Subtitle"};
constexpr LiteralStyleClass TitleBarGlyphStyleClass {"TitleBar/Glyph"};

constexpr LiteralStyleClass TitleBarIconStyleClass {"TitleBar/Icon"};
constexpr LiteralStyleClass TitleBarMinimizeButtonStyleClass {
  "TitleBar/MinimizeButton"};
constexpr LiteralStyleClass TitleBarMaximizeButtonStyleClass {
  "TitleBar/MaximizeButton"};
constexpr LiteralStyleClass TitleBarCloseButtonStyleClass {
  "TitleBar/CloseButton"};

constexpr std::string_view MinimizeGlyph = "\ue921";
constexpr std::string_view MaximizeGlyph = "\ue922";
constexpr std::string_view RestoreGlyph = "\ue923";
constexpr std::string_view CloseGlyph = "\ue8bb";

class TitleBarIconButton : public Button {
 public:
  using Button::Button;
  ~TitleBarIconButton() override = default;

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
  const auto pixelHeight = felly::numeric_cast<uint64_t>(
    renderer->GetPhysicalLength(destRect.GetHeight()));
  FUI_ASSERT(pixelHeight > 0);

  if (pixelHeight != mPreviousPixelHeight) {
    mTexture.reset();
  }
  if (!mTexture) {
    const auto bitmap = mProvider.GetBestSoftwareBitmap(pixelHeight);
    mTexture = renderer->ImportSoftwareBitmap(bitmap);
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

void TitleBar::SetIsActiveWindow(const bool isActive) {
  this->ToggleStyleClass(
    StaticTheme::TitleBar::TitleBarInactiveWindowStyleClass, !isActive);
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

void TitleBar::SetLeftWidgets(const std::vector<Widget*>& prepend) {
  for (auto&& it: prepend) {
    it->AddStyleClass(StaticTheme::TitleBar::TitleBarLeftButtonStyleClass);
  }
  auto widgets = prepend;
  widgets.append_range(
    std::array<Widget*, 3> {mIconButton, mTitleLabel, mSubtitleLabel});
  mContent->SetStructuralChildren(widgets);
}

TitleBar::TitleBar(Window* const window)
  : Widget(
      window,
      LiteralStyleClass("TitleBar"),
      StaticTheme::TitleBar::DefaultTitleBarStyle()) {
  using namespace StaticTheme::TitleBar;
  mContent = new Widget(
    window, ContentContainerStyleClass, TitleBarContentContainerStyle());
  auto chromeButtons = new Widget(window, ChromeButtonsContainerStyleClass, {});
  this->SetStructuralChildren({mContent, chromeButtons});

  mIconButton = new TitleBarIconButton(
    window, TitleBarIconStyleClass, TitleBarIconStyle(), {});
  mTitleLabel
    = new Label(window, TitleBarTitleStyleClass, TitleBarTitleStyle());
  mSubtitleLabel
    = new Label(window, TitleBarSubtitleStyleClass, TitleBarSubtitleStyle());
  this->SetLeftWidgets({});

  chromeButtons->SetStructuralChildren({
    mMinimizeButton = new Button(
      window,
      TitleBarMinimizeButtonStyleClass,
      WindowMinimizeMaximizeButtonStyle(),
      {}),
    mMaximizeButton = new Button(
      window,
      TitleBarMaximizeButtonStyleClass,
      WindowMinimizeMaximizeButtonStyle(),
      {}),
    mCloseButton = new Button(
      window, TitleBarCloseButtonStyleClass, WindowCloseButtonStyle(), {}),
  });
  mMinimizeButton->SetStructuralChildren({
    (new Label(window, TitleBarGlyphStyleClass, ImmutableStyle {}))
      ->SetText(MinimizeGlyph),
  });
  mMaximizeButton->SetStructuralChildren({
    mMaximizeLabel
    = (new Label(window, TitleBarGlyphStyleClass, ImmutableStyle {}))
        ->SetText(MaximizeGlyph),
  });
  mCloseButton->SetStructuralChildren(
    {(new Label(window, TitleBarGlyphStyleClass, ImmutableStyle {}))
       ->SetText(CloseGlyph)});
}

}// namespace FredEmmott::GUI::Widgets