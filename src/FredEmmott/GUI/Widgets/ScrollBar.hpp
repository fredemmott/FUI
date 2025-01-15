// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Orientation.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Label;

class ScrollBar final : public Widget {
 public:
  ScrollBar(std::size_t id, Orientation);
  ~ScrollBar();

  void SetMinimum(float value);
  [[nodiscard]] float GetMinimum() const;
  void SetMaximum(float value);
  [[nodiscard]] float GetMaximum() const;
  void SetValue(float value);
  [[nodiscard]] float GetValue() const;
  void SetThumbHeight(float value);
  [[nodiscard]] float GetThumbHeight() const;

 protected:
  [[nodiscard]]
  WidgetList GetDirectChildren() const noexcept override;
  [[nodiscard]]
  Style GetBuiltInStyles() const override;
  [[nodiscard]]
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  Orientation mOrientation;

  float mMinimum {0.0f};
  float mMaximum {1.0f};
  float mValue {0.0f};
  float mThumbHeight {0.0f};

  Style mBuiltinStyles;

  unique_ptr<Label> mSmallDecrement;// Arrow
  unique_ptr<Widget> mLargeDecrement;// Space above thumb
  unique_ptr<Widget> mThumb;
  unique_ptr<Widget> mLargeIncrement;// Space below thumb
  unique_ptr<Label> mSmallIncrement;// Arrow

  Style GetBuiltinStylesForOrientation() const;
};

}// namespace FredEmmott::GUI::Widgets