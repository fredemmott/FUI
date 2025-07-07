// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "Button.hpp"
#include "FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {

enum class ContentDialogButton {
  Primary,
  Secondary,
  Close,
};

void ContentDialogButton_AddAccent(ContentDialogButton);

template <ContentDialogButton T>
struct ContentDialogButtonResultMixin {
  template <class Self>
  decltype(auto) Accent(this Self&& self) {
    ContentDialogButton_AddAccent(T);
    return std::forward<Self>(self);
  }
};

}// namespace immediate_detail

void EndContentDialog();
using ContentDialogResult = Result<
  &EndContentDialog,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopedResultMixin>;

ContentDialogResult BeginContentDialog(
  ID id = ID {std::source_location::current()});

ContentDialogResult BeginContentDialog(
  bool* open,
  ID id = ID {std::source_location::current()});

void ContentDialogTitle(std::string_view title);
template <class... Args>
  requires(sizeof...(Args) > 0)
void ContentDialogTitle(std::format_string<Args...> fmt, Args&&... args) {
  ContentDialogTitle(std::format(fmt, std::forward<Args>(args)...));
}

void EndContentDialogButtons();
Result<&EndContentDialogButtons, void, immediate_detail::WidgetlessResultMixin>
BeginContentDialogButtons();

template <immediate_detail::ContentDialogButton T>
using ContentDialogButtonResult = Result<
  nullptr,
  bool,
  immediate_detail::ContentDialogButtonResultMixin<T>,
  immediate_detail::WidgetlessResultMixin>;

ContentDialogButtonResult<immediate_detail::ContentDialogButton::Primary>
ContentDialogPrimaryButton(std::string_view label);
ContentDialogButtonResult<immediate_detail::ContentDialogButton::Secondary>
ContentDialogSecondaryButton(std::string_view label);
ContentDialogButtonResult<immediate_detail::ContentDialogButton::Close>
ContentDialogCloseButton(std::string_view label = "Close");

}// namespace FredEmmott::GUI::Immediate