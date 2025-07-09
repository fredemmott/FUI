// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class RadioButton final : public Widget {
 public:
  explicit RadioButton(std::size_t id);
  ~RadioButton() override;

  [[nodiscard]]
  bool IsChecked() const noexcept {
    return mIsChecked;
  }

  void SetIsChecked(bool value) noexcept;

  bool mChanged {false};

 protected:
  Widget* GetFosterParent() const noexcept override;
  WidgetList GetDirectChildren() const noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
  [[nodiscard]] EventHandlerResult OnClick(const MouseEvent&) override;

 private:
  void SetStyles();
  void SetOuterStyles();
  void SetInnerStyles();
  void InitializeInnerStyles();

  bool mIsChecked {false};

  unique_ptr<Widget> mOuter;
  Widget* mInner {nullptr};
  unique_ptr<Widget> mFosterParent {};
};

}// namespace FredEmmott::GUI::Widgets