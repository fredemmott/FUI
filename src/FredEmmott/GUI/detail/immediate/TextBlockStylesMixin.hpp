// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Result.hpp>
#include <FredEmmott/GUI/StaticTheme/Generic.hpp>

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct TextBlockStylesMixin {
  template <class Self>
  decltype(auto) Caption(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::CaptionTextBlockClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Body(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::BodyTextBlockClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) BodyStrong(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::BodyStrongTextBlockClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Subtitle(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::SubtitleTextBlockClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Title(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::TitleTextBlockClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) TitleLarge(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::TitleLargeTextBlockClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) Display(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Generic::DisplayTextBlockClass);
    return std::forward<Self>(self);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail