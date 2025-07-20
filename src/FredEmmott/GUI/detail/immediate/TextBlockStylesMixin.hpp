// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Result.hpp>
#include <FredEmmott/GUI/StaticTheme/Generic.hpp>

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct TextBlockStylesMixin {
  template <class Self>
  decltype(auto) Caption(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::CaptionTextBlockStyle;
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Body(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::BodyTextBlockStyle;
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) BodyStrong(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::BodyStrongTextBlockStyle;
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Subtitle(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::SubtitleTextBlockStyle;
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Title(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::TitleTextBlockStyle;
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) TitleLarge(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::TitleLargeTextBlockStyle;
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Display(this Self&& self) {
    widget_from_result(self)->BuiltInStyles()
      += StaticTheme::Generic::DisplayTextBlockStyle;
    return std::forward<Self>(self);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail