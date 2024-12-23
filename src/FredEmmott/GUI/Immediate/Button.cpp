// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

void BeginButton(bool* clicked, const ButtonOptions& options) {
  BeginButton(
    clicked, options, MakeID<Widgets::Button>(tStack.back().mNextIndex));
}

void BeginButton(bool* clicked, const ButtonOptions& options, std::size_t id) {
  *clicked = false;
  TruncateUnlessNextIdEquals(id);

  auto& [siblings, i] = tStack.back();
  if (i == siblings.size()) {
    siblings.push_back(new Widgets::Button(id, options));
  }

  const auto child = static_cast<Widgets::Button*>(siblings.at(i))->GetChild();
  if (child) {
    tStack.emplace_back(std::vector {child});
  } else {
    tStack.emplace_back();
  }
}

void EndButton() {
  const auto back = std::move(tStack.back());
  if (back.mChildren.size() > 1) [[unlikely]] {
    throw std::logic_error("Buttons can only have a single child");
  }
  tStack.pop_back();

  auto& [siblings, i] = tStack.back();
  const auto button = static_cast<Widgets::Button*>(siblings.at(i));
  if (back.mChildren.empty()) {
    button->SetChild(nullptr);
  } else {
    button->SetChild(back.mChildren.back());
  }
  ++i;
}

}// namespace FredEmmott::GUI::Immediate