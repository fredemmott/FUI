// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Style.hpp"

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

  merge(&Style::mColor);
  merge(&Style::mBackgroundColor);
  merge(&Style::mBorderColor);

  merge(&Style::mFont);
  return *this;
}

}// namespace FredEmmott::GUI