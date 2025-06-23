// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <functional>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ScrollBarThumb final : public Widget {
 public:
  explicit ScrollBarThumb(std::size_t id);
  virtual ~ScrollBarThumb() override;

  void OnDrag(std::function<void(Point*)> callback);

 protected:
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;
  EventHandlerResult OnMouseMove(const MouseEvent&) override;

 private:
  std::function<void(Point*)> mOnDragCallback;
  std::optional<Point> mDragStart {};
};

}// namespace FredEmmott::GUI::Widgets