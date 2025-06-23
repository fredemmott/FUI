// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI::Widgets::widget_detail {

template <std::derived_from<Widget> T>
T* widget_cast(Widget* const p) {
  return dynamic_cast<T*>(p);
}

template <auto V>
struct constant_t {
  static constexpr auto value {V};
};

struct DetachedYogaTree {
  Widget* mLogicalParent {nullptr};
  Widget* mFosterParent {nullptr};
};

class PopupAnchorContext final : public Context {
 public:
  PopupAnchorContext() = delete;
  ~PopupAnchorContext() override = default;

  explicit PopupAnchorContext(Widget* anchor) : mAnchor(anchor) {}
  Widget* const mAnchor {nullptr};
};
static_assert(context<PopupAnchorContext>);

using YogaContext = std::variant<Widget*, DetachedYogaTree>;

}// namespace FredEmmott::GUI::Widgets::widget_detail
