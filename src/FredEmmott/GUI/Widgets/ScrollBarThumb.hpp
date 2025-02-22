// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <functional>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ScrollBarThumb final : public Widget {
 public:
  ScrollBarThumb(std::size_t id);
  virtual ~ScrollBarThumb() override;

  void OnDrag(std::function<void(SkPoint*)> callback);

 protected:
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;
  EventHandlerResult OnMouseMove(const MouseEvent&) override;

 private:
  std::function<void(SkPoint*)> mOnDragCallback;
  std::optional<SkPoint> mDragStart {};
};

}// namespace FredEmmott::GUI::Widgets