// Copyright 2025 Fred Emmott<fred @fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/utility/moved_flag.hpp>

namespace FredEmmott::GUI::Immediate::immediate_detail {
struct ResultOptions {
  const bool mHasWidgetPointer = true;
};

template <class T>
struct widget_from_result_t {};

template <class T>
  requires requires(T& v) { T::HasWidget; } && T::HasWidget
struct widget_from_result_t<T> {
  static auto operator()(const T& v) {
    return v.mWidget;
  };
};

template <class T>
  requires requires { widget_from_result_t<T> {}; }
auto widget_from_result(const T& v) {
  return widget_from_result_t<T> {}(v);
}

template <ResultOptions TOptions>
struct StyledResultMixin {};

template <ResultOptions TOptions>
  requires(TOptions.mHasWidgetPointer)
struct StyledResultMixin<TOptions> {
  template <class Self>
  decltype(auto) Styled(this Self&& self, const Style& style) {
    widget_from_result(self)->ReplaceExplicitStyles(style);
    return std::forward<Self>(self);
  }
};

template <void (*TEndWidget)()>
struct ScopedEndWidget {
  ScopedEndWidget(const ScopedEndWidget&) = delete;
  ScopedEndWidget& operator=(const ScopedEndWidget&) = delete;

  constexpr ScopedEndWidget() = default;
  constexpr ScopedEndWidget(ScopedEndWidget&&) noexcept = default;
  constexpr ScopedEndWidget& operator=(ScopedEndWidget&&) noexcept = default;

  constexpr ~ScopedEndWidget() {
    if (!mMoved) {
      TEndWidget();
    }
  }

 private:
  utility::moved_flag mMoved;
};

template <void (*TEndWidget)()>
struct ScopedResultMixin {
  auto Scoped() {
    if (std::exchange(mScoped, true)) [[unlikely]] {
      throw std::logic_error("Can't call Scoped() twice on the same Result");
    }
    return ScopedEndWidget<TEndWidget> {};
  }

 private:
  bool mScoped = false;
};
template <>
struct ScopedResultMixin<nullptr> {};

template <class TValue>
struct ValueResultMixin {
  constexpr TValue GetValue(this const auto& self) noexcept {
    return self.mValue;
  }

  constexpr operator TValue(this const auto& self) noexcept {
    return self.GetValue();
  }
};
template <>
struct ValueResultMixin<void> {};

}// namespace FredEmmott::GUI::Immediate::immediate_detail

namespace FredEmmott::GUI::Immediate {
template <
  void (*TEndWidget)() = nullptr,
  class TValue = void,
  immediate_detail::ResultOptions TOptions = {}>
class Result final : public immediate_detail::StyledResultMixin<TOptions>,
                     public immediate_detail::ScopedResultMixin<TEndWidget>,
                     public immediate_detail::ValueResultMixin<TValue> {
 public:
  static constexpr bool HasWidget = TOptions.mHasWidgetPointer;
  static constexpr bool HasValue = !std::is_void_v<TValue>;

  template <void (*)(), class, immediate_detail::ResultOptions>
  friend class Result;
  friend struct immediate_detail::ValueResultMixin<TValue>;
  friend std::conditional_t<
    HasWidget,
    immediate_detail::widget_from_result_t<Result>,
    void>;

  Result() = delete;

  Result()
    requires(!HasValue && !HasWidget)
  = default;

  template <std::convertible_to<TValue> T>
    requires(HasValue && !HasWidget)
  constexpr Result(T&& result) : mValue(std::forward<T>(result)) {}

  constexpr Result(Widgets::Widget* widget)
    requires(HasWidget && !HasValue)
    : mWidget(widget) {};

  template <void (*TOtherEndWidget)()>
    requires(HasWidget && !HasValue)
  constexpr Result(const Result<TOtherEndWidget, void>& other)
    : mWidget(other.mWidget) {}

  template <std::convertible_to<TValue> T>
    requires(HasWidget && HasValue)
  constexpr Result(Widgets::Widget* widget, T&& result)
    : mWidget(widget),
      mValue(std::forward<T>(result)) {}

  template <std::convertible_to<TValue> T, void (*TOtherEndWidget)()>
    requires(HasWidget && HasValue)
  constexpr Result(const Result<TOtherEndWidget, void>& other, T&& result)
    : mWidget(other.mWidget),
      mValue(std::forward<T>(result)) {}

 private:
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<HasWidget, Widgets::Widget*, std::monostate> mWidget {};
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<HasValue, TValue, std::monostate> mValue;
};
}// namespace FredEmmott::GUI::Immediate