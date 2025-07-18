// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ContentDialog.handwritten.hpp"

#include <FredEmmott/GUI/PseudoClasses.hpp>
#include <FredEmmott/GUI/StaticTheme/ContentDialog.hpp>
#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::ContentDialog {
using namespace StaticTheme::Common;
using namespace PseudoClasses;

const style_detail::lazy_init_style DefaultContentDialogStyle {[] {
  return Style()
    .BackgroundColor(ContentDialogBackground)
    .BorderColor(ContentDialogBorderBrush)
    .BorderRadius(OverlayCornerRadius)
    .BorderWidth(ContentDialogBorderWidth)
    .Color(ContentDialogForeground);
}};

}// namespace FredEmmott::GUI::StaticTheme::ContentDialog
