// Copyright 2025 Fred Emmott <fred@fredemmott.com>
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

}// namespace FredEmmott::GUI::Immediate::immediate_detail

namespace FredEmmott::GUI::Immediate {

template <
  void (*TEndWidget)() = nullptr,
  class TReturn = void,
  immediate_detail::ResultOptions TOptions = {}>
class Result final {
 public:
  static constexpr bool HasWidget = TOptions.mHasWidgetPointer;
  static constexpr bool HasValue = !std::is_void_v<TReturn>;

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

  template <void (*)(), class, immediate_detail::ResultOptions>
  friend class Result;
  friend std::conditional_t<
    HasWidget,
    immediate_detail::widget_from_result_t<Result>,
    void>;

  Result() = delete;

  Result()
    requires(!HasValue && !HasWidget)
  = default;

  constexpr Result(Widgets::Widget* widget)
    requires(HasWidget && !HasValue)
    : mWidget(widget) {};

  template <std::convertible_to<TReturn> T>
    requires(HasWidget && HasValue)
  constexpr Result(Widgets::Widget* widget, T&& result)
    : mWidget(widget),
      mResult(std::forward<T>(result)) {}

  template <std::convertible_to<TReturn> T>
    requires(HasValue && !HasWidget)
  constexpr Result(T&& result) : mResult(std::forward<T>(result)) {}

  template <void (*TOtherEndWidget)()>
    requires(HasWidget && !HasValue)
  constexpr Result(const Result<TOtherEndWidget, void>& other)
    : mWidget(other.mWidget) {}

  template <std::convertible_to<TReturn> T, void (*TOtherEndWidget)()>
    requires(HasWidget && HasValue)
  constexpr Result(const Result<TOtherEndWidget, void>& other, T&& result)
    : mWidget(other.mWidget),
      mResult(std::forward<T>(result)) {}

  constexpr TReturn GetValue() const noexcept
    requires(HasValue)
  {
    return mResult;
  }

  constexpr operator TReturn() const noexcept
    requires(HasValue)
  {
    return GetValue();
  }

  auto& Styled(const Style& style)
    requires HasWidget
  {
    mWidget->ReplaceExplicitStyles(style);
    return *this;
  }

  constexpr static auto Scoped()
    requires(TEndWidget != nullptr)
  {
    return ScopedEndWidget {};
  }

 private:
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<HasWidget, Widgets::Widget*, std::monostate> mWidget {};
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<HasValue, TReturn, std::monostate> mResult;
};
}// namespace FredEmmott::GUI::Immediate