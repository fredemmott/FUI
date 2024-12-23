// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

namespace FredEmmott::GUI::Immediate {

class Root final {
 public:
  void BeginFrame();
  void EndFrame(SkScalar w, SkScalar h, SkCanvas*);

 private:
  unique_ptr<Widgets::Widget> mWidget;

  void Paint(SkScalar w, SkScalar h, SkCanvas*) const;
};

}// namespace FredEmmott::GUI::Immediate