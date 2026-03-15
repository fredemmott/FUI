// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI::Widgets::widget_detail {

/** Cast a widget to a *known* subtype.
 *
 * If passed nullptr, returns nullptr.
 *
 * In debug builds, a non-null cast will be checked with RTTI.
 * In release builds, a non-null cast will be trusted.
 */
template <std::derived_from<Widget> T>
T* widget_cast(Widget* const p) {
  if constexpr (Config::Debug) {
    if (!p) {
      return nullptr;
    }
    const auto refined = dynamic_cast<T*>(p);
    FUI_ALWAYS_ASSERT(refined);
    return refined;
  } else {
    return static_cast<T*>(p);
  }
}

/** A compile-time constant.
 *
 * If `V` is invocable, it will be invoked, and the result will be used as the
 * value. This is a workaround for C++20 requirement that non-type template
 * parameter structs are structural types, in particular that they have no
 * private members.
 */
template <auto V>
struct constant_t {
 private:
  static consteval auto GetValue() {
    if constexpr (std::invocable<decltype(V)>) {
      return std::invoke(V);
    } else {
      return V;
    }
  }

 public:
  static constexpr auto value = GetValue();
};

class PopupAnchorContext final : public Context {
 public:
  PopupAnchorContext() = delete;
  ~PopupAnchorContext() override = default;

  explicit PopupAnchorContext(Widget* anchor) : mAnchor(anchor) {}
  Widget* const mAnchor {nullptr};
};
static_assert(context<PopupAnchorContext>);

}// namespace FredEmmott::GUI::Widgets::widget_detail
