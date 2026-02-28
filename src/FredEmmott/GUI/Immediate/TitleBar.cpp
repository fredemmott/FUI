// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "TitleBar.hpp"

#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

void WindowTitle(const std::string_view title) {
  immediate_detail::tWindow->SetTitle(title);
}

bool WindowSubtitle(const std::string_view title) {
  return immediate_detail::tWindow->SetSubtitle(title);
}

}// namespace FredEmmott::GUI::Immediate