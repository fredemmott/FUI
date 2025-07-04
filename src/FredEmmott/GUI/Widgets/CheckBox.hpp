// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>
#include <FredEmmott/GUI/Widgets/Label.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class CheckBox final : public Widget {
 public:
  explicit CheckBox(std::size_t id);

  // Indeterminate (half-checked) checkboxes are not currently supported.
  [[nodiscard]]
  bool IsChecked() const noexcept;
  void SetIsChecked(bool) noexcept;

  ActivatedFlag mChanged;

 protected:
  Style GetBuiltInStyles() const override;
  EventHandlerResult OnClick(const MouseEvent& event) override;
  Widget* GetFosterParent() const noexcept override;
  WidgetList GetDirectChildren() const noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  // Using an enum here to make things clearer if support for Indeterminate
  // is added in the future
  enum class State {
    Checked,
    Unchecked,
  };
  State mState {State::Unchecked};

  std::unique_ptr<Widget> mCheckGlyphBackground;
  Label* mCheckGlyph {nullptr};
  std::unique_ptr<Widget> mFosterParent;

  void UpdateCheckGlyphStyles();
};

}// namespace FredEmmott::GUI::Widgets
