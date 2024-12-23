// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

#define FUI_STYLE_PROPERTIES(X) \
  X(BackgroundColor) \
  X(BorderColor) \
  X(BorderRadius) \
  X(BorderWidth) \
  X(Color) \
  X(Font) \
  X(Height) \
  X(Margin) \
  X(Padding) \
  X(PaddingBottom) \
  X(PaddingLeft) \
  X(PaddingRight) \
  X(PaddingTop) \
  X(Width)

namespace FredEmmott::GUI {

Style& Style::operator+=(const Style& other) noexcept {
  /* Set the lhs to the rhs, if the rhs is set.
   * e.g. with:
   *
   * ```css
   * myclass {
   *   color: white;
   *   background-color: black;
   * }
   * myclass:hover {
   *   background-color: red;
   * }
   * ```
   *
   * `myclass + myclass:hover` should result in:
   *
   *  ```
   *  {
   *    color: white;
   *    background-color: red;
   *  }
   *  ```
   */
  const auto merge = [this, &other](auto member) {
    auto& lhs = this->*member;
    const auto& rhs = other.*member;

    if (rhs.has_value()) {
      lhs = rhs;
    }
  };

#define MERGE_PROPERTY(X) merge(&Style::m##X);
  FUI_STYLE_PROPERTIES(MERGE_PROPERTY)
#undef MERGE_PROPERTIES

  return *this;
}

Style Style::InheritableValues() const noexcept {
  Style ret;
  const auto copyIfInheritable = [this, &ret](auto member) {
    const auto& value = this->*member;
    using T = std::decay_t<decltype(value)>;
    if constexpr (std::same_as<T, InheritableValue<typename T::value_type>>) {
      ret.*member = value;
    }
  };
#define COPY_IF_INHERITABLE(X) copyIfInheritable(&Style::m##X);
  FUI_STYLE_PROPERTIES(COPY_IF_INHERITABLE)
#undef COPY_IF_INHERITABLE
  return ret;
}

}// namespace FredEmmott::GUI