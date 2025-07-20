// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ContentDialog.hpp"

#include "FredEmmott/GUI/Immediate/Label.hpp"
#include "FredEmmott/GUI/StaticTheme/ContentDialog.hpp"
#include "FredEmmott/GUI/Widgets/Button.hpp"
#include "FredEmmott/GUI/Widgets/Label.hpp"
#include "FredEmmott/GUI/Widgets/PopupWindow.hpp"
#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate/widget_from_result.hpp"
#include "PopupWindow.hpp"
#include "PushID.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;
using namespace StaticTheme::Common;
using namespace StaticTheme::ContentDialog;

namespace {
struct Context : Widgets::Context {
  std::string mTitleText {};
  Widgets::Label* mTitleLabel {nullptr};
};
thread_local Context* tContext {nullptr};

struct ButtonsContext : Widgets::Context {
  static constexpr uint8_t VisibleFlag = 1;
  static constexpr uint8_t DisabledFlag = 2;
  static constexpr uint8_t AccentFlag = 4;

  struct LayoutButton {
    Widgets::Button* mButton {nullptr};
    Widgets::Label* mLabel {nullptr};
  };
  struct LogicalButton {
    std::string mText;
    uint8_t mFlags {};
    LayoutButton* mBinding {nullptr};
  };

  Widget* mOuter {nullptr};

  LayoutButton mFirstColumn;
  Widget* mFirstSpacer {nullptr};
  LayoutButton mSecondColumn;
  Widget* mSecondSpacer {nullptr};
  LayoutButton mThirdColumn;

  LogicalButton mPrimary {"Primary"};
  LogicalButton mSecondary {"Secondary"};
  LogicalButton mClose {"Close"};
};
thread_local ButtonsContext* tButtonsContext {nullptr};
}// namespace

void EndContentDialog() {
  tContext = nullptr;
  EndVStackPanel();
  EndBasicPopupWindow();
}

ContentDialogResult BeginContentDialog(const ID id) {
  FUI_ASSERT(!tContext, "ContentDialogs can not be nested");
  if (!BeginBasicPopupWindow(id).Transparent().Modal()) {
    return false;
  }

  static const Style OuterStyle = DefaultContentDialogStyle + Style().Gap(0);
  const auto outer = BeginVStackPanel().Styled(OuterStyle);
  tContext = widget_from_result(outer)->GetOrCreateContext<Context>();

  static const auto InnerStyle
    = Style()
        .BackgroundColor(ContentDialogTopOverlay)
        .BorderBottomWidth(ContentDialogSeparatorThicknessBottom)
        .BorderColor(ContentDialogSeparatorBorderBrush)
        .BorderLeftWidth(ContentDialogSeparatorThicknessLeft)
        .BorderRightWidth(ContentDialogSeparatorThicknessRight)
        .BorderTopWidth(ContentDialogSeparatorThicknessTop)
        .PaddingBottom(ContentDialogPaddingBottom)
        .PaddingLeft(ContentDialogPaddingLeft)
        .PaddingRight(ContentDialogPaddingRight)
        .PaddingTop(ContentDialogPaddingTop);
  BeginVStackPanel().Styled(InnerStyle);

  static const auto TitleStyle
    = Style()
        .Font(
          Font {WidgetFont::ControlContent}.WithSize(20).WithWeight(
            FontWeight::SemiBold))
        .MarginBottom(ContentDialogTitleMarginBottom)
        .MarginLeft(ContentDialogTitleMarginLeft)
        .MarginRight(ContentDialogTitleMarginRight)
        .MarginTop(ContentDialogTitleMarginTop);
  Label(tContext->mTitleText).Styled(TitleStyle);
  tContext->mTitleLabel = GetCurrentNode<Widgets::Label>();
  return true;
}

ContentDialogResult BeginContentDialog(bool* open, const ID id) {
  if (!(open && *open)) {
    return false;
  }

  *open = BeginContentDialog(id);
  return *open;
}

void ContentDialogTitle(std::string_view title) {
  FUI_ASSERT(tContext);
  if (tContext->mTitleText == title) {
    return;
  }
  tContext->mTitleText = std::string {title};
  tContext->mTitleLabel->SetText(title);
}

template <class T>
auto GetButtonWidget(const T& v) {
  if constexpr (std::convertible_to<Widgets::Widget*, T>) {
    return v;
  }
  if constexpr (std::convertible_to<ButtonsContext::LayoutButton*, T>) {
    return v->mButton;
  }
  if constexpr (std::convertible_to<ButtonsContext::LayoutButton&, T>) {
    return v.mButton;
  }
  std::unreachable();
};

void Hide(auto w) {
  static const auto Hide = Style().Display(YGDisplayNone);
  GetButtonWidget(w)->AddExplicitStyles(Hide);
}
auto Show(auto w) {
  static const auto Show = Style().Display(YGDisplayFlex);
  GetButtonWidget(w)->AddExplicitStyles(Show);
};

void SingleButton(ButtonsContext::LogicalButton& b) {
  const auto ctx = tButtonsContext;
  Hide(ctx->mFirstColumn);
  Hide(ctx->mFirstSpacer);
  Hide(ctx->mSecondColumn);
  Hide(ctx->mSecondSpacer);
  Show(ctx->mThirdColumn);

  ctx->mPrimary.mBinding = nullptr;
  ctx->mSecondary.mBinding = nullptr;
  ctx->mClose.mBinding = nullptr;
  b.mBinding = &ctx->mThirdColumn;
};

void EndContentDialogButtons() {
  FUI_ASSERT(tButtonsContext);
  const auto clearOnScopeExit
    = wil::scope_exit([&ctx = tButtonsContext] { ctx = nullptr; });
  auto ctx = tButtonsContext;

  unsigned int visibleButtons = 0;
  constexpr unsigned int Primary = 1;
  constexpr unsigned int Secondary = 2;
  constexpr unsigned int Close = 4;

  if ((tButtonsContext->mPrimary.mFlags & ButtonsContext::VisibleFlag)) {
    visibleButtons |= Primary;
  }
  if ((tButtonsContext->mSecondary.mFlags & ButtonsContext::VisibleFlag)) {
    visibleButtons |= Secondary;
  }
  if ((tButtonsContext->mClose.mFlags & ButtonsContext::VisibleFlag)) {
    visibleButtons |= Close;
  }
  if (visibleButtons == 0) {
    Hide(ctx->mOuter);
    return;
  }
  Show(ctx->mOuter);

  switch (visibleButtons) {
    case Primary | Secondary | Close:
      Show(ctx->mFirstColumn);
      Show(ctx->mFirstSpacer);
      Show(ctx->mSecondColumn);
      Show(ctx->mSecondSpacer);
      Show(ctx->mThirdColumn);

      ctx->mPrimary.mBinding = &ctx->mFirstColumn;
      ctx->mSecondary.mBinding = &ctx->mSecondColumn;
      ctx->mClose.mBinding = &ctx->mThirdColumn;
      break;
    case Primary:
      SingleButton(ctx->mPrimary);
      break;
    case Secondary:
      SingleButton(ctx->mSecondary);
      break;
    case Close:
      SingleButton(ctx->mClose);
      break;
    case Primary | Secondary:
      Show(ctx->mFirstColumn);
      Hide(ctx->mFirstSpacer);
      Hide(ctx->mSecondColumn);
      Show(ctx->mSecondSpacer);
      Show(ctx->mThirdColumn);

      ctx->mPrimary.mBinding = &ctx->mFirstColumn;
      ctx->mSecondary.mBinding = &ctx->mThirdColumn;
      ctx->mClose.mBinding = nullptr;
      break;
    case Primary | Close:
      Show(ctx->mFirstColumn);
      Hide(ctx->mFirstSpacer);
      Hide(ctx->mSecondColumn);
      Show(ctx->mSecondSpacer);
      Show(ctx->mThirdColumn);

      ctx->mPrimary.mBinding = &ctx->mFirstColumn;
      ctx->mSecondary.mBinding = nullptr;
      ctx->mClose.mBinding = &ctx->mThirdColumn;
      break;
    case Secondary | Close:
      Hide(ctx->mFirstColumn);
      Hide(ctx->mFirstSpacer);
      Show(ctx->mSecondColumn);
      Show(ctx->mSecondSpacer);
      Show(ctx->mThirdColumn);

      ctx->mPrimary.mBinding = nullptr;
      ctx->mSecondary.mBinding = &ctx->mSecondColumn;
      ctx->mClose.mBinding = &ctx->mThirdColumn;
      break;
  }

  for (auto&& button: {ctx->mPrimary, ctx->mSecondary, ctx->mClose}) {
    if (!button.mBinding) {
      continue;
    }
    const auto it = button.mBinding;
    it->mLabel->SetText(button.mText);
    it->mButton->SetIsDirectlyDisabled(
      button.mFlags & ButtonsContext::DisabledFlag);
    it->mButton->ReplaceExplicitStyles(
      (button.mFlags & ButtonsContext::AccentFlag)
        ? StaticTheme::Button::AccentButtonStyle
        : Style {});
  }
}

Result<&EndContentDialogButtons, void, WidgetlessResultMixin>
BeginContentDialogButtons() {
  if (tContext) {
    EndVStackPanel();
  }
  static const auto OuterStyle
    = Style()
        .BackgroundColor(ContentDialogBackground)
        .BorderBottomLeftRadius(OverlayCornerRadius)
        .BorderBottomRightRadius(OverlayCornerRadius)
        .Gap(0)
        .PaddingBottom(ContentDialogPaddingBottom)
        .PaddingLeft(ContentDialogPaddingLeft)
        .PaddingRight(ContentDialogPaddingRight)
        .PaddingTop(ContentDialogPaddingTop);
  const auto ctx = tButtonsContext
    = GetCurrentParentNode()->GetOrCreateContext<ButtonsContext>();
  const auto outer = BeginHStackPanel().Styled(OuterStyle).Scoped();
  ctx->mOuter = GetCurrentParentNode();
  ctx->mPrimary.mFlags = {};
  ctx->mSecondary.mFlags = {};
  ctx->mClose.mFlags = {};

  const auto button = [](
                        ButtonsContext::LayoutButton& it,
                        const ID& id = ID {std::source_location::current()}) {
    it.mButton = BeginWidget<Widgets::Button>(id);
    it.mLabel = BeginWidget<Widgets::Label>(ID {0});
    it.mLabel->ReplaceExplicitStyles(Style().FlexGrow(1.0));
    EndWidget<Widgets::Label>();
    EndWidget<Widgets::Button>();

    it.mButton->BuiltInStyles() += Style().FlexGrow(1);
  };
  const auto spacer = [](const ID& id = ID {std::source_location::current()}) {
    const auto w = BeginWidget<Widgets::Widget>(id);
    w->BuiltInStyles() = Style().Width(ContentDialogButtonSpacing);
    EndWidget<Widgets::Widget>();
    return w;
  };

  button(ctx->mFirstColumn);
  ctx->mFirstSpacer = spacer();
  button(ctx->mSecondColumn);
  ctx->mSecondSpacer = spacer();
  button(ctx->mThirdColumn);
  return {};
}

static constexpr auto GetButtonProjection(ContentDialogButton button) {
  using enum ContentDialogButton;
  switch (button) {
    case Primary:
      return &ButtonsContext::mPrimary;
    case Secondary:
      return &ButtonsContext::mSecondary;
    case Close:
      return &ButtonsContext::mClose;
  }
  std::unreachable();
}

static auto& GetButton(ContentDialogButton button) {
  FUI_ASSERT(
    tButtonsContext,
    "ContentDialog buttons can only be used inside a ContentDialogButtons "
    "scope");
  return std::invoke(GetButtonProjection(button), tButtonsContext);
}

template <ContentDialogButton TButton>
static ContentDialogButtonResult<TButton> ContentDialogButtonImpl(
  std::string_view title) {
  auto& ctx = GetButton(TButton);
  ctx.mFlags |= ButtonsContext::VisibleFlag;
  ctx.mText = std::string {title};

  if (GetCurrentParentNode()->IsDisabled()) {
    ctx.mFlags |= ButtonsContext::DisabledFlag;
  }

  const bool clicked
    = ctx.mBinding && std::exchange(ctx.mBinding->mButton->mClicked, false);
  if (tContext && clicked) {
    ClosePopupWindow();
  }
  return clicked;
}

ContentDialogButtonResult<ContentDialogButton::Primary>
ContentDialogPrimaryButton(std::string_view label) {
  return ContentDialogButtonImpl<ContentDialogButton::Primary>(label);
}

ContentDialogButtonResult<ContentDialogButton::Secondary>
ContentDialogSecondaryButton(std::string_view label) {
  return ContentDialogButtonImpl<ContentDialogButton::Secondary>(label);
}

ContentDialogButtonResult<ContentDialogButton::Close> ContentDialogCloseButton(
  std::string_view label) {
  return ContentDialogButtonImpl<ContentDialogButton::Close>(label);
}

void immediate_detail::ContentDialogButton_AddAccent(
  ContentDialogButton button) {
  GetButton(button).mFlags |= ButtonsContext::AccentFlag;
  ;
}

}// namespace FredEmmott::GUI::Immediate