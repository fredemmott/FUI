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

enum class StylePropertyPriority {
  UserAgentBaseline,
  Inherited,
  UserAgent,
  Normal,
  Important,
};

enum class Cursor {
  Default,
  Pointer,// Hand
  Text,// I-beam cursor
};

enum class Overflow {
  Visible,
  Hidden,
  Scroll,
};

enum class Display {
  Flex,
  None,
  Contents,
};

enum class FlexDirection {
  Column,
  ColumnReverse,
  Row,
  RowReverse,
};

enum class Align {
  Auto,
  FlexStart,
  Center,
  FlexEnd,
  Stretch,
  Baseline,
  SpaceBetween,
  SpaceAround,
  SpaceEvenly,
};

enum class BoxSizing {
  BorderBox,
  ContentBox,
};

}// namespace FredEmmott::GUI
