// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "CheckBox.hpp"

#include <FredEmmott/GUI/StaticTheme/CheckBox.hpp>
#include <FredEmmott/GUI/Widgets/Label.hpp>

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "WidgetList.hpp"

#ifdef FUI_ENABLE_DIRECT2D
#include "FredEmmott/GUI/Direct2DRenderer.hpp"
#endif

#ifdef FUI_ENABLE_SKIA
#include "FredEmmott/GUI/SkiaRenderer.hpp"
#endif

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass CheckBoxStyleClass {"CheckBox"};
constexpr LiteralStyleClass CheckGlyphStyleClass {"CheckBox/CheckGlyph"};
constexpr LiteralStyleClass FosterParentStyleClass {"CheckBox/FosterParent"};

auto MakeCheckBoxStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  const Style DefaultCheckBoxStyle
    = Style()
        .BackgroundColor(CheckBoxBackgroundUnchecked)
        .BorderColor(CheckBoxBorderBrushUnchecked)
        .BorderRadius(ControlCornerRadius)
        .BorderWidth(CheckBoxBorderThickness)
        .Color(CheckBoxForegroundUnchecked)
        .Font(WidgetFont::ControlContent)
        .Height(CheckBoxHeight)
        .MarginLeft(-CheckBoxPadding.GetLeft())
        .MinWidth(CheckBoxMinWidth)
        .Padding(CheckBoxPadding)
        .OutlineLeftOffset(7)
        .OutlineTopOffset(3)
        .OutlineRightOffset(7)
        .OutlineBottomOffset(3);

  const auto& UncheckedNormal = DefaultCheckBoxStyle;
  const Style UncheckedPointerOver
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedPointerOver)
        .BorderColor(CheckBoxBorderBrushUncheckedPointerOver)
        .Color(CheckBoxForegroundUncheckedPointerOver);
  const Style UncheckedPressed
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedPressed)
        .BorderColor(CheckBoxBorderBrushUncheckedPressed)
        .Color(CheckBoxForegroundUncheckedPressed);
  const Style UncheckedDisabled
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedDisabled)
        .BorderColor(CheckBoxBorderBrushUncheckedDisabled)
        .Color(CheckBoxForegroundUncheckedDisabled);
  const Style UncheckedStyle = UncheckedNormal
    + Style()
        .And(Hover, UncheckedPointerOver)
        .And(Active, UncheckedPressed)
        .And(Disabled, UncheckedDisabled);
  const Style CheckedNormal = DefaultCheckBoxStyle
    + Style()
        .BackgroundColor(CheckBoxBackgroundChecked)
        .BorderColor(CheckBoxBorderBrushChecked)
        .Color(CheckBoxForegroundChecked);
  const Style CheckedPointerOver
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedPointerOver)
        .BorderColor(CheckBoxBorderBrushCheckedPointerOver)
        .Color(CheckBoxForegroundCheckedPointerOver);
  const Style CheckPressed
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedPressed)
        .BorderColor(CheckBoxBorderBrushCheckedPressed)
        .Color(CheckBoxForegroundCheckedPressed);
  const Style CheckedDisabled
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedDisabled)
        .BorderColor(CheckBoxBorderBrushCheckedDisabled)
        .Color(CheckBoxForegroundCheckedDisabled);
  const Style CheckedStyle = CheckedNormal
    + Style()
        .And(Hover, CheckedPointerOver)
        .And(Active, CheckPressed)
        .And(Disabled, CheckedDisabled);

  return Style().And(Checked, CheckedStyle).And(!Checked, UncheckedStyle);
}

auto& CheckBoxStyles() {
  static const ImmutableStyle ret {MakeCheckBoxStyles()};
  return ret;
}

auto& FosterParentStyles() {
  using namespace StaticTheme::CheckBox;
  static const ImmutableStyle ret {
    Style()
      .PaddingBottom(CheckBoxPadding.GetBottom() + 6)
      .PaddingLeft(CheckBoxPadding.GetLeft())
      .PaddingRight(CheckBoxPadding.GetRight())
      .PaddingTop(CheckBoxPadding.GetTop()),
  };
  return ret;
}

auto MakeGlyphStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  const auto DefaultGlyphStyles
    = Style()
        .BorderRadius(ControlCornerRadius)
        .BorderWidth(CheckBoxBorderThickness)
        .Font(
          SystemFont::ResolveGlyphFont(SystemFont::BodyStrong)
            .WithSize(CheckBoxGlyphSize),
          !important)
        .Height(CheckBoxSize)
        .Width(CheckBoxSize);
  const Style& UncheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUnchecked)
        .BorderColor(CheckBoxCheckBackgroundStrokeUnchecked)
        .Color(CheckBoxCheckGlyphForegroundUnchecked);
  const Style UncheckedPointerOverGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedPointerOver)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedPointerOver)
        .Color(CheckBoxCheckGlyphForegroundUncheckedPointerOver);
  const Style UncheckedPointerPressedGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedPressed)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedPressed)
        .Color(CheckBoxCheckGlyphForegroundUncheckedPressed);
  const Style UncheckedDisabledGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedDisabled)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedDisabled)
        .Color(CheckBoxCheckGlyphForegroundUncheckedDisabled);
  const Style UncheckedGlyphStyles = UncheckedNormalGlyphStyles
    + Style()
        .And(Hover, UncheckedPointerOverGlyphStyles)
        .And(Active, UncheckedPointerPressedGlyphStyles)
        .And(Disabled, UncheckedDisabledGlyphStyles);
  const Style& CheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillChecked)
        .BorderColor(CheckBoxCheckBackgroundStrokeChecked)
        .Color(CheckBoxCheckGlyphForegroundChecked);
  const Style CheckedPointerOverGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedPointerOver)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedPointerOver)
        .Color(CheckBoxCheckGlyphForegroundCheckedPointerOver);
  const Style CheckedPointerPressedGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedPressed)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedPressed)
        .Color(CheckBoxCheckGlyphForegroundCheckedPressed);
  const Style CheckedDisabledGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedDisabled)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedDisabled)
        .Color(CheckBoxCheckGlyphForegroundCheckedDisabled);
  const Style CheckedGlyphStyles = CheckedNormalGlyphStyles
    + Style()
        .And(Hover, CheckedPointerOverGlyphStyles)
        .And(Active, CheckedPointerPressedGlyphStyles)
        .And(Disabled, CheckedDisabledGlyphStyles);

  return Style()
    .And(Checked, CheckedGlyphStyles)
    .And(!Checked, UncheckedGlyphStyles);
}

auto& GlyphStyles() {
  static const ImmutableStyle ret {MakeGlyphStyles()};
  return ret;
}

class CheckBoxGlyph final : public Widget {
 public:
  CheckBoxGlyph() : Widget(0, CheckGlyphStyleClass, GlyphStyles()) {}
  ~CheckBoxGlyph() override = default;

 protected:
  void Tick(const std::chrono::steady_clock::time_point& now) override;
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;
  [[nodiscard]]
  FrameRateRequirement GetFrameRateRequirement() const noexcept override {
    if (mAnimationFinishedAt > std::chrono::steady_clock::now()) {
      return FrameRateRequirement::SmoothAnimation {};
    }
    return Widget::GetFrameRateRequirement();
  }

 private:
  // Derived from AnimatedAcceptVisualSource.cpp, which is a lottie-generated
  // file
  static constexpr auto Scale
    = (StaticTheme::CheckBox::CheckBoxGlyphSize - 1) / (48.0f * 0.7f);
  static constexpr auto Thickness = 4.0f * Scale;
  static constexpr auto P1 = Point {-15.172f, 0.016f} * Scale;
  static constexpr auto P2 = Point {-5.0f, 10.188f} * Scale;
  static constexpr auto P3 = Point {15.337, -10.337} * Scale;
  static constexpr auto L12 = P1.DistanceTo(P2);
  static constexpr auto L23 = P2.DistanceTo(P3);
  static constexpr auto L123 = L12 + L23;
  static constexpr auto L12_Unit = (P2 - P1) * (1.0f / L12);
  static constexpr auto L23_Unit = (P3 - P2) * (1.0f / L23);

  static constexpr auto Ease
    = EasingFunctions::CubicBezier(0.167f, 0.167f, 0.833f, 0.833f);

  std::chrono::steady_clock::time_point mTickedAt {};
  std::chrono::steady_clock::time_point mAnimationFinishedAt {};

  [[nodiscard]]
  float GetPartialStrokeLength(std::chrono::steady_clock::time_point now) const;

#ifdef FUI_ENABLE_DIRECT2D
  void PaintOwnContent(Direct2DRenderer*, const Rect&, const Brush&) const;
  ID2D1PathGeometry* GetCompleteDirect2DPath(Direct2DRenderer*) const;
#endif
#ifdef FUI_ENABLE_SKIA
  void PaintOwnContent(SkiaRenderer*, const Rect&, const Brush&) const;
  const SkPath& GetCompleteSkiaPath() const;
#endif
};

void CheckBoxGlyph::Tick(const std::chrono::steady_clock::time_point& now) {
  if (!IsChecked()) {
    mTickedAt = {};
  } else if (mTickedAt == decltype(mTickedAt) {}) {
    mTickedAt = now;
    mAnimationFinishedAt
      = now + StaticTheme::Common::ControlFastAnimationDuration;
  }
}

void CheckBoxGlyph::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  if (!GetStructuralParent<CheckBox>()->IsChecked()) {
    return;
  }
  renderer->PushLayer();
  renderer->Translate(rect.GetWidth() / 2, (rect.GetHeight() / 2) + 1);
  const auto pop = felly::scope_exit([renderer] { renderer->PopLayer(); });

  const auto brush = style.Color().value();
#ifdef FUI_ENABLE_DIRECT2D
  if (const auto p = direct2d_renderer_cast(renderer)) {
    PaintOwnContent(p, rect, brush);
    return;
  }
#endif
#ifdef FUI_ENABLE_SKIA
  if (const auto p = skia_renderer_cast(renderer)) {
    PaintOwnContent(p, rect, brush);
    return;
  }
#endif
  FUI_FATAL("Unsupported renderer for CheckBoxGlyph");
}

float CheckBoxGlyph::GetPartialStrokeLength(
  const std::chrono::steady_clock::time_point now) const {
  const auto length
    = Ease(
        std::chrono::duration_cast<std::chrono::duration<float>>(
          now - mTickedAt)
        / (mAnimationFinishedAt - mTickedAt))
    * L123;
  return length;
}
#ifdef FUI_ENABLE_DIRECT2D
void CheckBoxGlyph::PaintOwnContent(
  Direct2DRenderer* renderer,
  const Rect& rect,
  const Brush& brush) const {
  const auto d2dBrush = brush.as<ID2D1Brush*>(renderer, rect);

  const auto& resources = renderer->GetDeviceResources();
  const auto d2d = resources.mD2DDeviceContext;

  const auto now = std::chrono::steady_clock::now();
  if (now >= mAnimationFinishedAt) {
    d2d->DrawGeometry(
      GetCompleteDirect2DPath(renderer),
      d2dBrush,
      Thickness,
      renderer->GetStrokeStyle(StrokeCap::Round));
    return;
  }

  const auto length = GetPartialStrokeLength(now);

  using win32_detail::CheckHResult;
  wil::com_ptr<ID2D1PathGeometry> path;
  CheckHResult(resources.mD2DFactory->CreatePathGeometry(path.put()));
  wil::com_ptr<ID2D1GeometrySink> sink;
  CheckHResult(path->Open(sink.put()));
  sink->BeginFigure(P1.as<D2D1_POINT_2F>(), D2D1_FIGURE_BEGIN_FILLED);
  if (length < L12) {
    const auto end = P1 + (L12_Unit * length);
    sink->AddLine(end.as<D2D1_POINT_2F>());
  } else {
    sink->AddLine(P2.as<D2D1_POINT_2F>());
    const auto end = P2 + (L23_Unit * (length - L12));
    sink->AddLine(end.as<D2D1_POINT_2F>());
  }
  sink->EndFigure(D2D1_FIGURE_END_OPEN);
  CheckHResult(sink->Close());
  d2d->DrawGeometry(
    path.get(),
    d2dBrush,
    Thickness,
    renderer->GetStrokeStyle(StrokeCap::Round));
}

ID2D1PathGeometry* CheckBoxGlyph::GetCompleteDirect2DPath(
  Direct2DRenderer* const renderer) const {
  // ID2D1PathGeometry is a device-independent resource; if we move to
  // ID2D1GeometryRealization, we need smarter caching
  thread_local wil::com_ptr<ID2D1PathGeometry> path;
  if (path) {
    return path.get();
  }

  using win32_detail::CheckHResult;
  CheckHResult(
    renderer->GetDeviceResources().mD2DFactory->CreatePathGeometry(path.put()));

  wil::com_ptr<ID2D1GeometrySink> sink;
  CheckHResult(path->Open(sink.put()));
  sink->BeginFigure(P1.as<D2D1_POINT_2F>(), D2D1_FIGURE_BEGIN_FILLED);
  sink->AddLine(P2.as<D2D1_POINT_2F>());
  sink->AddLine(P3.as<D2D1_POINT_2F>());
  sink->EndFigure(D2D1_FIGURE_END_OPEN);
  CheckHResult(sink->Close());
  return path.get();
}
#endif

#ifdef FUI_ENABLE_SKIA
void CheckBoxGlyph::PaintOwnContent(
  SkiaRenderer* renderer,
  const Rect& rect,
  const Brush& brush) const {
  const auto canvas = renderer->GetSkCanvas();
  auto paint = brush.as<SkPaint>(renderer, rect);
  paint.setAntiAlias(true);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(Thickness);
  paint.setStrokeCap(SkPaint::kRound_Cap);
  paint.setStrokeJoin(SkPaint::kRound_Join);

  const auto now = std::chrono::steady_clock::now();
  if (now >= mAnimationFinishedAt) {
    canvas->drawPath(GetCompleteSkiaPath(), paint);
    return;
  }

  const auto length = GetPartialStrokeLength(now);

  SkPath path;
  path.moveTo(P1.mX, P1.mY);
  if (length < L12) {
    const auto [x, y] = P1 + (L12_Unit * length);
    path.lineTo(x, y);
  } else {
    path.lineTo(P2.mX, P2.mY);
    const auto [x, y] = P2 + (L23_Unit * (length - L12));
    path.lineTo(x, y);
  }
  canvas->drawPath(path, paint);
}

const SkPath& CheckBoxGlyph::GetCompleteSkiaPath() const {
  thread_local SkPath p {};
  if (!p.isEmpty()) {
    return p;
  }
  p.moveTo(P1.mX, P1.mY);
  p.lineTo(P2.mX, P2.mY);
  p.lineTo(P3.mX, P3.mY);
  return p;
}
#endif

}// namespace

CheckBox::CheckBox(id_type id)
  : Widget(
      id,
      LiteralStyleClass {"CheckBox"},
      CheckBoxStyles(),
      {*CheckBoxStyleClass}),
    IToggleable(this) {
  this->SetStructuralChildren({
    mGlyph = new CheckBoxGlyph(),
    mFosterParent = new Widget(0, FosterParentStyleClass, FosterParentStyles()),
  });
}

void CheckBox::Toggle() {
  SetIsChecked(!IsChecked());
  mWasChanged = true;
}

Widget::EventHandlerResult CheckBox::OnClick(const MouseEvent&) {
  this->Toggle();
  return EventHandlerResult::StopPropagation;
}

Widget* CheckBox::GetStructuralParentForLogicalChildren() noexcept {
  return mFosterParent;
}

Widget::ComputedStyleFlags CheckBox::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
