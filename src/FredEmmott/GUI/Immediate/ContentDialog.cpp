// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ContentDialog.hpp"

#include "FredEmmott/GUI/Immediate/Label.hpp"
#include "FredEmmott/GUI/StaticTheme/ContentDialog.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "PopupWindow.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;
using namespace StaticTheme::ContentDialog;

void EndContentDialog() {
  EndVStackPanel();
  EndBasicPopupWindow();
}

ContentDialogResult BeginContentDialog(
  [[maybe_unused]] const std::string_view title,
  const ID id) {
  if (!BeginBasicPopupWindow(id).Transparent()) {
    return false;
  }

  BeginVStackPanel().Styled(DefaultContentDialogStyle + Style {.mGap = 0});

  BeginVStackPanel().Styled(
    Style {
      .mBackgroundColor = ContentDialogTopOverlay,
      .mBorderBottomWidth = ContentDialogSeparatorThicknessBottom,
      .mBorderColor = ContentDialogSeparatorBorderBrush,
      .mBorderLeftWidth = ContentDialogSeparatorThicknessLeft,
      .mBorderRightWidth = ContentDialogSeparatorThicknessRight,
      .mBorderTopWidth = ContentDialogSeparatorThicknessTop,
      .mPaddingBottom = ContentDialogPaddingBottom,
      .mPaddingLeft = ContentDialogPaddingLeft,
      .mPaddingRight = ContentDialogPaddingRight,
      .mPaddingTop = ContentDialogPaddingTop,
    });

  static const auto TitleFont
    = Font {WidgetFont::ControlContent}.WithSize(20).WithWeight(
      FontWeight::SemiBold);
  Label(title).Styled(
    Style {
      .mFont = TitleFont,
      .mMarginBottom = ContentDialogTitleMarginBottom,
      .mMarginLeft = ContentDialogTitleMarginLeft,
      .mMarginRight = ContentDialogTitleMarginRight,
      .mMarginTop = ContentDialogTitleMarginTop,
    });

  return true;
}

ContentDialogResult
BeginContentDialog(bool* open, const std::string_view title, const ID id) {
  if (!(open && *open)) {
    return false;
  }

  *open = BeginContentDialog(title, id);
  return *open;
}

void EndContentDialogFooter() {
  EndHStackPanel();
}

Result<&EndContentDialogFooter, void, WidgetlessResultMixin>
BeginContentDialogFooter() {
  EndVStackPanel();// content

  BeginHStackPanel().Styled({
    .mBackgroundColor = ContentDialogBackground,
    .mPaddingBottom = ContentDialogPaddingBottom,
    .mPaddingLeft = ContentDialogPaddingLeft,
    .mPaddingRight = ContentDialogPaddingRight,
    .mPaddingTop = ContentDialogPaddingTop,
  });
  return {};
}

}// namespace FredEmmott::GUI::Immediate