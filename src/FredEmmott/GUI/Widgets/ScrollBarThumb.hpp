// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <functional>

#include "FredEmmott/GUI/Orientation.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ScrollBarThumb final : public Widget {
 public:
  explicit ScrollBarThumb(Orientation, id_type id);
  ~ScrollBarThumb() override;

  void OnDrag(std::function<void(Point*)> callback);
  void OnDrop(std::function<void(Point)> callback);

 protected:
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;
  EventHandlerResult OnMouseMove(const MouseEvent&) override;

 private:
  std::function<void(Point*)> mOnDragCallback;
  std::function<void(Point)> mOnDropCallback;
  std::optional<Point> mDragStart {};
};

}// namespace FredEmmott::GUI::Widgets