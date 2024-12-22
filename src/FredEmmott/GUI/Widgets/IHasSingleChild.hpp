// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::Widgets {

class Widget;

class IHasSingleChild {
  public:
   virtual ~IHasSingleChild() = default;
   virtual Widget* GetChild() const noexcept = 0;
   virtual void SetChild(Widget*) = 0;
};

}