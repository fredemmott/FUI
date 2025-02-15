// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/StyleSheet.hpp>

namespace FredEmmott::GUI {

StyleClass ScrollBarStyleClass();
StyleClass ScrollBarHorizontalStyleClass();
StyleClass ScrollBarVerticalStyleClass();
StyleClass ScrollBarSmallChangeStyleClass();
StyleClass ScrollBarSmallDecrementStyleClass();
StyleClass ScrollBarSmallIncrementStyleClass();
StyleClass ScrollBarThumbStyleClass();
StyleClass ScrollBarLargeChangeStyleClass();
StyleClass ScrollBarLargeDecrementStyleClass();
StyleClass ScrollBarLargeIncrementStyleClass();

StyleSheet ScrollBarStyles();
StyleSheet ScrollBarSmallChangeStyles();
StyleSheet ScrollBarLargeChangeStyles();
StyleSheet ScrollBarThumbStyles();

}// namespace FredEmmott::GUI