// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI {
enum class PointerEvents {
  Auto,
  None,
};

enum class TextAlign {
  Left,
  Center,
  Right,
};

enum class StylePropertyScope {
  Self,
  SelfAndChildren,
  SelfAndDescendants,
};

enum class Cursor {
  Default,
  Pointer,// Hand
};

}// namespace FredEmmott::GUI
