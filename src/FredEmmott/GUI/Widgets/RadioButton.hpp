// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class RadioButton final : public Widget {
 public:
  explicit RadioButton(std::size_t id);
  ~RadioButton() override;

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  bool mChanged {false};

 protected:
  Widget* GetFosterParent() const noexcept override;
  WidgetList GetDirectChildren() const noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
  [[nodiscard]] EventHandlerResult OnClick(const MouseEvent&) override;

 private:
  void InitializeOuterStyles();
  void InitializeInnerStyles();

  unique_ptr<Widget> mOuter;
  Widget* mInner {nullptr};
  unique_ptr<Widget> mFosterParent {};
};

}// namespace FredEmmott::GUI::Widgets