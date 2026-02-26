// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <d3d11.h>
#include <d3d11_4.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <atomic>
#include <map>
#include <print>

#include "FredEmmott/GUI/detail/win32_detail.hpp"

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

constexpr auto LoremIpsum
  = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. 💩 Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";

namespace {
struct TextureProducer {
  wil::unique_handle mTexture {};
  wil::unique_handle mFence {};
  std::atomic<uint64_t> mFenceValue {};
  bool mEarlySignal {false};

  static constexpr fui::Size Size {128, 128};

  void WaitUntilReady() {
    mReady.wait(false);
  }

  TextureProducer() {
    mThread = std::jthread {std::bind_front(&TextureProducer::Run, this)};
  }

 private:
  std::atomic_flag mReady;
  std::jthread mThread;

  void Run(const std::stop_token& stop) {
    using fui::win32_detail::CheckHResult;
    wil::com_ptr<ID3D11Device> basicDevice;
    wil::com_ptr<ID3D11DeviceContext> basicContext;
    CheckHResult(D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
        | D3D11_CREATE_DEVICE_DEBUG
#endif
      ,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      basicDevice.put(),
      nullptr,
      basicContext.put()));
    const auto device = basicDevice.query<ID3D11Device5>();
    const auto context = basicContext.query<ID3D11DeviceContext4>();

    wil::com_ptr<ID3D11Fence> fence;
    CheckHResult(device->CreateFence(
      0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(fence.put())));
    CheckHResult(
      fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, mFence.put()));

    wil::com_ptr<ID3D11Texture2D1> texture;
    constexpr D3D11_TEXTURE2D_DESC1 desc {
      .Width = static_cast<DWORD>(Size.mWidth),
      .Height = static_cast<DWORD>(Size.mHeight),
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .SampleDesc = {1, 0},
      .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
      .MiscFlags
      = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED,
    };
    CheckHResult(device->CreateTexture2D1(&desc, nullptr, texture.put()));
    const auto resource = texture.query<IDXGIResource1>();
    CheckHResult(resource->CreateSharedHandle(
      nullptr, DXGI_SHARED_RESOURCE_READ, nullptr, mTexture.put()));

    wil::com_ptr<ID3D11RenderTargetView> rtv;
    CheckHResult(
      device->CreateRenderTargetView(texture.get(), nullptr, rtv.put()));

    static constexpr float Red[4] {1.f, 0.f, 0.f, 1.f};
    static constexpr float Green[4] {0.f, 1.f, 0.f, 1.f};
    static constexpr float White[4] {1.f, 1.f, 1.f, 1.f};

    while (!stop.stop_requested()) {
      const auto start = std::chrono::steady_clock::now();
      if (!mReady.test_and_set()) {
        mReady.notify_all();
      }
      ++mFenceValue;

      const auto offset
        = (std::chrono::duration_cast<std::chrono::milliseconds>(
             start.time_since_epoch())
             .count()
           / 10)
        % std::llround(Size.mWidth);
      const D3D11_RECT line {
        static_cast<LONG>(offset),
        0,
        static_cast<LONG>(offset + 1),
        static_cast<LONG>(Size.mHeight),
      };

      context->ClearRenderTargetView(rtv.get(), Red);
      context->ClearView(rtv.get(), White, &line, 1);
      if (mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      context->ClearRenderTargetView(rtv.get(), Green);
      context->ClearView(rtv.get(), White, &line, 1);
      if (!mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }

      const auto elapsed = std::chrono::steady_clock::now() - start;
      std::this_thread::sleep_for(std::chrono::milliseconds(100) - elapsed);
    }
  }
};
struct SwapChainPusher {
  fui::Widgets::SwapChainPanel::SwapChain mSwapChain;
  wil::unique_handle mFence {};
  std::atomic<uint64_t> mFenceValue {};
  bool mEarlySignal {false};

  static constexpr fui::Size Size {128, 128};

  SwapChainPusher(fui::Widgets::SwapChainPanel::SwapChain swapChain)
    : mSwapChain(std::move(swapChain)) {
    mThread = std::jthread {std::bind_front(&SwapChainPusher::Run, this)};
  }

 private:
  std::jthread mThread;

  void Run(const std::stop_token& stop) {
    using fui::win32_detail::CheckHResult;
    wil::com_ptr<ID3D11Device> basicDevice;
    wil::com_ptr<ID3D11DeviceContext> basicContext;
    CheckHResult(D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
        | D3D11_CREATE_DEVICE_DEBUG
#endif
      ,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      basicDevice.put(),
      nullptr,
      basicContext.put()));
    const auto device = basicDevice.query<ID3D11Device5>();
    const auto context = basicContext.query<ID3D11DeviceContext4>();

    wil::com_ptr<ID3D11Fence> fence;
    CheckHResult(device->CreateFence(
      0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(fence.put())));
    CheckHResult(
      fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, mFence.put()));
    bool fenceIsNew = true;

    static constexpr float Red[4] {1.f, 0.f, 0.f, 1.f};
    static constexpr float Green[4] {0.f, 1.f, 0.f, 1.f};
    static constexpr float White[4] {1.f, 1.f, 1.f, 1.f};
    while (!stop.stop_requested()) {
      const auto begin = mSwapChain.BeginFrame();
      if (!begin) {
        return;
      }

      const auto start = std::chrono::steady_clock::now();
      wil::com_ptr<ID3D11Texture2D> texture;
      CheckHResult(device->OpenSharedResource1(
        begin->mTexture, IID_PPV_ARGS(texture.put())));
      wil::com_ptr<ID3D11RenderTargetView> rtv;
      CheckHResult(
        device->CreateRenderTargetView(texture.get(), nullptr, rtv.put()));

      ++mFenceValue;

      const auto offset
        = (std::chrono::duration_cast<std::chrono::milliseconds>(
             start.time_since_epoch())
             .count()
           / 10)
        % std::llround(Size.mWidth);
      const D3D11_RECT line {
        static_cast<LONG>(offset),
        0,
        static_cast<LONG>(offset + 1),
        static_cast<LONG>(Size.mHeight),
      };

      context->ClearRenderTargetView(rtv.get(), Red);
      context->ClearView(rtv.get(), White, &line, 1);
      if (mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      context->ClearRenderTargetView(rtv.get(), Green);
      context->ClearView(rtv.get(), White, &line, 1);
      if (!mEarlySignal) {
        CheckHResult(context->Signal(fence.get(), mFenceValue));
      }

      const fui::Widgets::SwapChainPanel::SwapChain::EndFrameInfo end {
        .mFence = mFence.get(),
        .mFenceValue = mFenceValue,
        .mFenceIsNew = std::exchange(fenceIsNew, false),
      };
      mSwapChain.EndFrame(*begin, end);

      const auto elapsed = std::chrono::steady_clock::now() - start;
      std::this_thread::sleep_for(std::chrono::milliseconds(100) - elapsed);
    }
  }
};
}// namespace

static void AppTick(fui::Window& window) {
  constexpr bool UseScrollView = true;
  if constexpr (UseScrollView) {
    using enum fui::Window::ResizeMode;
    window.SetResizeMode(AllowGrow, AllowShrink);
    fuii::BeginVScrollView().Styled(
      fui::Style()
        .BackgroundColor(
          fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush)
        .FlexGrow(1));
  }

  fuii::BeginVStackPanel().Styled(
    fui::Style().FlexGrow(1).Gap(12).Margin(12).Padding(8));

  fuii::Label("FUI Details").Subtitle();
  {
    // Could call EndCard() and EndVStackPanel() instead of .Scoped();
    const auto card = fuii::BeginCard().Scoped();
    const auto layout = fuii::BeginVStackPanel().Scoped();
    fuii::Label(
      "Backend: {} ({} ICU)",
      fui::GetBackendDescription(),
#ifdef FUI_ENABLE_ICU
      "bundled"
#else
      "Windows"
#endif
    );
    fuii::Label("_WIN32_WINNT: {:#010X}", _WIN32_WINNT);
    fuii::Label("NTDDI_VERSION: {:#010X}", NTDDI_VERSION);
  }

  fuii::Label("Controls").Subtitle();
  fuii::BeginCard();
  fuii::BeginVStackPanel().Styled(fui::Style().FlexGrow(1));

  static bool sDisableAll = false;
  // (void) cast to ignore [[nodiscard]] is-changed return value
  fuii::ToggleSwitch(&sDisableAll).Caption("Disable all controls");
  fuii::BeginDisabled(sDisableAll);

  static bool sIsChecked {false};
  if (fuii::CheckBox(&sIsChecked, "I'm a checkbox!")
        .ToolTip("CheckBox ToolTip")) {
    std::println(
      stderr, "Checkbox changed to {}", sIsChecked ? "checked" : "unchecked");
  }

  fuii::Label("Hello, world; this text doesn't make the button wider aeg");

  static bool popupVisible = false;
  if (fuii::Button("Click Me!")
        .Caption("Button")
        .Styled(fui::Style().MinWidth(200.f))) {
    popupVisible = true;
    std::println(stderr, "Clicked!");
  }
  if (auto tt = fuii::BeginToolTipForPreviousWidget().Scoped()) {
    fuii::Label("This is a tooltip");
  }

  if (const auto popup = fuii::BeginPopup(&popupVisible).Scoped()) {
    const auto card = fuii::BeginCard().Scoped();
    const auto layout = fuii::BeginVStackPanel().Scoped();
    fuii::Label("This is a popup");
    if (fuii::Button("Close")) {
      popupVisible = false;
    }
  }

  static bool sShowAccentPopup = false;
  if (fuii::Button("Accent style")
        .Caption("Accent button")
        .Accent()
        .ToolTip("Fluent ToolTip")) {
    std::println(stderr, "Accent clicked");
    sShowAccentPopup = true;
  }
  if (
    const auto dialog = fuii::BeginContentDialog(&sShowAccentPopup).Scoped()) {
    fuii::ContentDialogTitle("Demo dialog title");
    fuii::Label("This is a ContentDialog.");
    fuii::TextBlock(LoremIpsum).Styled(fui::Style().MinWidth(400));

    const auto footer = fuii::BeginContentDialogButtons().Scoped();
    if (fuii::ContentDialogPrimaryButton("Test").Accent()) {
      std::println(stderr, "ContentDialog primary button clicked");
    }
    fuii::BeginDisabled();
    fuii::ContentDialogSecondaryButton("Test Disabled");
    fuii::EndDisabled();
    if (fuii::ContentDialogCloseButton()) {
      std::println(stderr, "ContentDialog close button clicked");
    }
  }

  static bool isOn = true;
  if (fuii::ToggleSwitch(&isOn).Caption("ToggleSwitch")) {
    std::println(stderr, "Toggled to {}", isOn);
  }
  (void)(fuii::ToggleSwitch(&isOn)
           .Caption("ToggleSwitch with custom on/off text (discouraged)")
           .OnText("Foo (on)")
           .OffText("Bar (off)"));

  {
    static int selectedIndex = 1;
    constexpr auto comboItems = std::array {
      "foo",
      "bar",
      "baz",
    };
    if (fuii::ComboBox(&selectedIndex, comboItems)
          .Caption("Array of strings")
          .ToolTip("CB ToolTip")) {
      std::println(stderr, "Combo changed to {}", comboItems[selectedIndex]);
    }
  }
  {
    struct Item {
      std::string_view mLabel;
    };
    static int selectedIndex = 1;
    constexpr auto comboItems = std::array {
      Item {"a"},
      Item {"b"},
      Item {"c"},
    };
    (void)fuii::ComboBox(&selectedIndex, comboItems, &Item::mLabel)
      .Caption("Array of structs");
  }
  {
    struct Item {
      int mKey;
      std::string_view mLabel;
    };
    static int selectedIndex = 1;
    constexpr auto comboItems = std::array {
      Item {123, "a"},
      Item {456, "b"},
      Item {789, "c"},
    };
    if (fuii::ComboBox(&selectedIndex, comboItems, &Item::mLabel, &Item::mKey)
          .Caption("Key-value structs")) {
      std::println(stderr, "value is {}", selectedIndex);
    }
  }
  {
    static int selectedIndex = 1;
    const auto comboItems = std::map<int, std::string_view> {
      {123, "Foo"},
      {456, "Bar"},
      {789, "echo echo echo"},
    };
    if (fuii::ComboBox(&selectedIndex, comboItems).Caption("std::map")) {
      std::println(stderr, "value is {}", selectedIndex);
    }
  }

  static std::size_t selectedOption = 1;
  fuii::BeginRadioButtons("Radio Buttons Header");
  for (std::size_t i = 0; i < 3; ++i) {
    if (fuii::RadioButton(&selectedOption, i, "Option {}", i)) {
      std::println(stderr, "Radio changed to {}", i);
    }
  }
  fuii::EndRadioButtons();

  fuii::BeginHStackPanel();
  // Glyphs areIUnicode private usage code points from
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  fuii::FontIcon("\ueb51");// Heart
  fuii::FontIcon("\ueb52");// HeartFill
  fuii::FontIcon({
    {"\ueb52", fui::Style().Color(fui::Colors::Red)},
    {"\ueb51"},
  });
  fuii::Label("After stack");
  fuii::EndStackPanel();

  fuii::TextBlock(LoremIpsum);

  for (int i = 0; i < 3; ++i) {
    // IDs are automatically generated for formats
    fuii::Label("Formatted label {}", i);
  }
  for (int i = 0; i < 3; ++i) {
    // ... but not if we're passing in a variable like below,
    // so we need to explicitly manage state
    const auto scopedID = fuii::PushID(i).Scoped();
    fuii::Label(std::format("String label {}", i));
  }

  if (fuii::HyperlinkButton("I'm a HyperlinkButton!")) {
    std::println(stderr, "Hyperlink clicked");
  }

  static std::string text {"Hello 💩 world"};
  if (fuii::TextBox(&text).Caption("I'm a TextBox!")) {
    std::println(stderr, "TextBox changed to {}", text);
  }

  static float sliderValue {0};
  fuii::HSlider(&sliderValue).TickFrequency(25);
  fuii::VSlider(&sliderValue)
    .TickFrequency(25)
    .SnapToTicks()
    .Styled(fui::Style().Height(120));

  static std::optional<int> numberBoxValue;
  fuii::NumberBox(&numberBoxValue);

  fuii::EndDisabled();

  static TextureProducer textureSource;
  textureSource.WaitUntilReady();
  fuii::ToggleSwitch(&textureSource.mEarlySignal).Caption("Early fence signal");
  fuii::GPUTexture(
    fui::ImportedTexture::HandleKind::NTHandle,
    textureSource.mTexture.get(),
    textureSource.mFence.get(),
    textureSource.mFenceValue,
    fui::Rect {fui::Point {}, TextureProducer::Size})
    .Styled(
      fui::Style()
        .Width(TextureProducer::Size.mWidth)
        .Height(TextureProducer::Size.mHeight));
  auto swapChainPanel = fuii::SwapChainPanel().Styled(
    fui::Style()
      .Width(SwapChainPusher::Size.mWidth)
      .Height(SwapChainPusher::Size.mHeight));
  static SwapChainPusher swapChainPusher {swapChainPanel.GetSwapChain()};
  swapChainPusher.mEarlySignal = textureSource.mEarlySignal;

  fuii::EndStackPanel();
  fuii::EndCard();
  fuii::EndStackPanel();
  if constexpr (UseScrollView) {
    fuii::EndVScrollView();
  }
}

int WINAPI wWinMain(
  const HINSTANCE hInstance,
  const HINSTANCE hPrevInstance,
  const LPWSTR lpCmdLine,
  const int nCmdShow) {
  return fui::Win32Window::WinMain(
    hInstance,
    hPrevInstance,
    lpCmdLine,
    nCmdShow,
    [](fui::Win32Window& window) { AppTick(window); },
    {"FUI Demo"});
}
