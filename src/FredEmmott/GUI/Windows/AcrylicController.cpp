// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "AcrylicController.hpp"

#include <DirectXMath.h>
#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1helper.h>
#include <dwmapi.h>
#include <wil/winrt.h>
#include <windows.graphics.effects.h>
#include <windows.graphics.effects.interop.h>
#include <windows.system.h>
#include <windows.ui.composition.desktop.h>
#include <windows.ui.composition.interop.h>
#include <wrl.h>

#include <random>

#include "FredEmmott/GUI/Brush.hpp"
#include "FredEmmott/GUI/SystemSettings.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "FredEmmott/utility/almost_equal.hpp"

namespace FredEmmott::GUI {
namespace {
using namespace win32_detail;
namespace WUC = ABI::Windows::UI::Composition;
namespace WGE = ABI::Windows::Graphics::Effects;

wil::unique_hstring MakeHSTRING(const std::wstring_view in) {
  wil::unique_hstring ret;
  CheckHResult(WindowsCreateString(in.data(), in.size(), ret.put()));
  return ret;
}

HRESULT MakeHSTRING(const std::wstring_view in, HSTRING* out) {
  return WindowsCreateString(in.data(), in.size(), out);
}

auto GetPropertyValueFactory() {
  thread_local const auto ret = wil::GetActivationFactory<
    ABI::Windows::Foundation::IPropertyValueStatics>(
    RuntimeClass_Windows_Foundation_PropertyValue);
  return ret.get();
}

template <class T>
struct PropertyValueConverter;

template <>
struct PropertyValueConverter<float> {
  static auto Convert(const float value) {
    wil::com_ptr<IInspectable> ret;
    CheckHResult(GetPropertyValueFactory()->CreateSingle(value, ret.put()));
    return ret.query<ABI::Windows::Foundation::IPropertyValue>();
  }
};

template <>
struct PropertyValueConverter<uint32_t> {
  static auto Convert(const uint32_t value) {
    wil::com_ptr<IInspectable> ret;
    CheckHResult(GetPropertyValueFactory()->CreateUInt32(value, ret.put()));
    return ret.query<ABI::Windows::Foundation::IPropertyValue>();
  }
};

template <std::size_t N>
struct PropertyValueConverter<float[N]> {
  static auto Convert(const float arr[N]) {
    wil::com_ptr<IInspectable> ret;
    CheckHResult(
      GetPropertyValueFactory()->CreateSingleArray(
        N, const_cast<float*>(arr), ret.put()));
    return ret.query<ABI::Windows::Foundation::IPropertyValue>();
  }

  // Using a template U so that this can be easily extended to support other
  // matrix types in the future
  template <class U>
    requires(sizeof(U) == sizeof(std::array<float, N>))
    && std::same_as<D2D1::Matrix5x4F, U>
  static auto Convert(const U& m) {
    const auto& arr = *reinterpret_cast<const std::array<float, N>*>(&m);
    return Convert(arr.data());
  }
};

template <>
struct PropertyValueConverter<bool> {
  static auto Convert(const bool value) {
    wil::com_ptr<IInspectable> ret;
    CheckHResult(
      GetPropertyValueFactory()->CreateBoolean(
        value ? TRUE : FALSE, ret.put()));
    return ret.query<ABI::Windows::Foundation::IPropertyValue>();
  }
};

template <class T>
auto MakePropertyValue(auto&& value) {
  using Converter = PropertyValueConverter<T>;
  return Converter::Convert(std::forward<decltype(value)>(value));
}

auto MakeCompositionEffectSourceParameter(const std::wstring_view name) {
  thread_local const auto factory
    = wil::GetActivationFactory<WUC::ICompositionEffectSourceParameterFactory>(
      RuntimeClass_Windows_UI_Composition_CompositionEffectSourceParameter);
  wil::com_ptr<WUC::ICompositionEffectSourceParameter> ret;
  CheckHResult(factory->Create(MakeHSTRING(name).release(), ret.put()));
  return ret.query<WGE::IGraphicsEffectSource>();
}

/** Effect for use with Windows::UI::Composition
 *
 * The documented approach is to use Win2D to create these effects, however,
 * it's only available via nuget, and nuget is not an acceptable dependency,
 * especially as it would be transitive to all library users.
 *
 * Fortunately, as far as WUC is concerned, the Win2D effects are just a
 * description of Direct2D effects. We can implement the required interfaces
 * ourselves to avoid that dependency.
 */
struct CompositionEffect
  : Microsoft::WRL::RuntimeClass<
      Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
      WGE::IGraphicsEffect,
      WGE::IGraphicsEffectSource,
      WGE::IGraphicsEffectD2D1Interop> {
  ~CompositionEffect() override = default;

  HRESULT put_Name(HSTRING) override {
    return S_OK;
  }

  STDMETHOD(GetNamedPropertyMapping)(
    LPCWSTR,
    UINT*,
    ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping)
    override {
    *mapping = ABI::Windows::Graphics::Effects::
      GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT;
    return S_OK;
  }

  STDMETHOD(GetPropertyCount)(UINT* count) override {
    *count = Properties().size();
    return S_OK;
  }

  STDMETHOD(GetProperty)(
    UINT index,
    ABI::Windows::Foundation::IPropertyValue** value) override {
    if (index >= Properties().size()) {
      return E_INVALIDARG;
    }
    Properties().at(index).mValue.copy_to(value);
    return S_OK;
  }

  STDMETHOD(
    GetSource)(const UINT index, WGE::IGraphicsEffectSource** source) override {
    Sources().at(index).copy_to(source);
    return S_OK;
  }

  STDMETHOD(GetSourceCount)(UINT* count) override {
    *count = Sources().size();
    return S_OK;
  }

  template <class T>
  void CreateBrush(WUC::ICompositor* compositor, T&& out) {
    wil::com_ptr<WUC::ICompositionEffectFactory> factory;
    CheckHResult(compositor->CreateEffectFactory(this, factory.put()));
    CheckHResult(factory->CreateBrush(std::forward<T>(out)));
  }

  [[nodiscard]]
  auto CreateBrush(WUC::ICompositor* compositor) {
    wil::com_ptr<WUC::ICompositionEffectBrush> ret;
    CreateBrush(compositor, ret.put());
    return ret;
  }

 protected:
  using IPropertyValue = ABI::Windows::Foundation::IPropertyValue;
  struct Property {
    std::wstring_view mName;
    wil::com_ptr<IPropertyValue> mValue;
  };
  using PropertyMap = std::vector<Property>;
  using SourceList = std::vector<wil::com_ptr<WGE::IGraphicsEffectSource>>;

  virtual PropertyMap& Properties() = 0;
  virtual SourceList& Sources() = 0;

  template <class T>
  void SetPropertyByName(const std::wstring_view name, auto&& value) {
    if (name.empty()) {
      return;
    }
    const auto it = std::ranges::find(Properties(), name, &Property::mName);
    it->mValue = MakePropertyValue<T>(std::forward<decltype(value)>(value));
  }
};

struct CompositionBlurEffect final : CompositionEffect {
  ~CompositionBlurEffect() override = default;
  HRESULT get_Name(HSTRING* name) override {
    return MakeHSTRING(L"Blur", name);
  }

  STDMETHOD(GetEffectId)(GUID* id) override {
    *id = CLSID_D2D1GaussianBlur;
    return S_OK;
  }

  void SetBlurAmount(const float value) {
    SetPropertyByName<float>(L"BlurAmount", value);
  }
  void SetBorderMode(const D2D1_BORDER_MODE mode) {
    SetPropertyByName<uint32_t>(L"BorderMode", mode);
  }

 protected:
  PropertyMap& Properties() override {
    return mProperties;
  }
  SourceList& Sources() override {
    return mSources;
  }

 private:
  PropertyMap mProperties {
    {L"BlurAmount", MakePropertyValue<float>(3.0f)},
    {L"Optimization",
     MakePropertyValue<uint32_t>(D2D1_GAUSSIANBLUR_OPTIMIZATION_BALANCED)},
    {L"BorderMode", MakePropertyValue<uint32_t>(D2D1_BORDER_MODE_SOFT)},
  };

  SourceList mSources {
    MakeCompositionEffectSourceParameter(L"source"),
  };
};

struct CompositionBlendEffect final : CompositionEffect {
  ~CompositionBlendEffect() override = default;
  HRESULT get_Name(HSTRING* name) override {
    return MakeHSTRING(L"Blend", name);
  }

  STDMETHOD(GetEffectId)(GUID* id) override {
    *id = CLSID_D2D1Blend;
    return S_OK;
  }

  void SetMode(const D2D1_BLEND_MODE mode) {
    SetPropertyByName<uint32_t>(L"Mode", mode);
  }

 protected:
  PropertyMap& Properties() override {
    return mProperties;
  }
  SourceList& Sources() override {
    return mSources;
  }

 private:
  PropertyMap mProperties {
    {L"Mode", MakePropertyValue<uint32_t>(D2D1_BLEND_MODE_MULTIPLY)},
  };
  SourceList mSources {
    // Win2D names seem inverted, so using the Direct2D names for the inputs
    // https://github.com/microsoft/Win2D/issues/958
    MakeCompositionEffectSourceParameter(L"source"),
    MakeCompositionEffectSourceParameter(L"destination"),
  };
};

struct CompositionCompositeEffect final : CompositionEffect {
  ~CompositionCompositeEffect() override = default;
  HRESULT get_Name(HSTRING* name) override {
    return MakeHSTRING(L"Composite", name);
  }

  STDMETHOD(GetEffectId)(GUID* id) override {
    *id = CLSID_D2D1Composite;
    return S_OK;
  }

 protected:
  PropertyMap& Properties() override {
    return mProperties;
  }
  SourceList& Sources() override {
    return mSources;
  }

 private:
  PropertyMap mProperties {
    {L"Mode", MakePropertyValue<uint32_t>(D2D1_COMPOSITE_MODE_SOURCE_OVER)},
  };
  SourceList mSources {
    MakeCompositionEffectSourceParameter(L"destination"),
    MakeCompositionEffectSourceParameter(L"source"),
  };
};

struct CompositionColorMatrixEffect final : CompositionEffect {
  ~CompositionColorMatrixEffect() override = default;
  HRESULT get_Name(HSTRING* name) override {
    return MakeHSTRING(L"ColorMatrix", name);
  }

  STDMETHOD(GetEffectId)(GUID* id) override {
    *id = CLSID_D2D1ColorMatrix;
    return S_OK;
  }

  void SetColorMatrix(const D2D1::Matrix5x4F& matrix) {
    SetPropertyByName<float[20]>(L"ColorMatrix", matrix);
  }

 protected:
  PropertyMap& Properties() override {
    return mProperties;
  }
  SourceList& Sources() override {
    return mSources;
  }

 private:
  PropertyMap mProperties {
    {L"ColorMatrix", MakePropertyValue<float[20]>(D2D1::Matrix5x4F {})},
    {L"AlphaMode", MakePropertyValue<uint32_t>(D2D1_ALPHA_MODE_PREMULTIPLIED)},
    {L"ClampOutput", MakePropertyValue<bool>(false)},
  };
  SourceList mSources {
    MakeCompositionEffectSourceParameter(L"source"),
  };
};

struct CompositionBorderEffect final : CompositionEffect {
  ~CompositionBorderEffect() override = default;

  HRESULT get_Name(HSTRING* name) override {
    return MakeHSTRING(L"Border", name);
  }

  STDMETHOD(GetEffectId)(GUID* id) override {
    *id = CLSID_D2D1Border;
    return S_OK;
  }

  void SetExtendX(const D2D1_BORDER_EDGE_MODE mode) {
    SetPropertyByName<uint32_t>(L"ExtendX", mode);
  }
  void SetExtendY(const D2D1_BORDER_EDGE_MODE mode) {
    SetPropertyByName<uint32_t>(L"ExtendY", mode);
  }

 protected:
  PropertyMap& Properties() override {
    return mProperties;
  }
  SourceList& Sources() override {
    return mSources;
  }

 private:
  PropertyMap mProperties {
    {L"ExtendX", MakePropertyValue<uint32_t>(D2D1_BORDER_EDGE_MODE_CLAMP)},
    {L"ExtendY", MakePropertyValue<uint32_t>(D2D1_BORDER_EDGE_MODE_CLAMP)},
  };

  SourceList mSources {
    MakeCompositionEffectSourceParameter(L"source"),
  };
};

// Based on WinUI3 AcrylicBrush::GetLuminosityColor
Color::Constant GetLuminosityColor(
  const Color::Constant& color,
  const std::optional<float> luminosityOpacity) {
  const auto [r, g, b, a] = color.GetRGBAFTuple();
  if (luminosityOpacity) {
    return Color::Constant::FromRGBA128F(
      r, g, b, std::clamp(*luminosityOpacity, 0.0f, 1.0f));
  }

  static constexpr auto MinV = 0.125f;
  static constexpr auto MaxV = 0.965f;
  static constexpr auto MinAlpha = 0.125f;
  static constexpr auto MaxAlpha = 1.03f;
  static constexpr auto AlphaScale = 1.0f / (MaxAlpha - MinAlpha);

  using namespace DirectX;
  const auto rgbaIn = XMVectorSet(r, g, b, 1.0f);
  auto hsva = XMColorRGBToHSV(rgbaIn);
  // HSV Value is clamped
  hsva = XMVectorSetZ(hsva, std::clamp(XMVectorGetZ(hsva), MinV, MaxV));
  const auto rgbaOut = XMColorHSVToRGB(hsva);

  return Color::Constant::FromRGBA128F(
    XMVectorGetX(rgbaOut),
    XMVectorGetY(rgbaOut),
    XMVectorGetZ(rgbaOut),
    (a * AlphaScale) + MinAlpha);
}

// Based on WinUI3 AcrylicBrush::GetTintOpacityModifier
Color::Constant WithTintModifier(const Color::Constant& color) {
  static constexpr auto MidPoint = 0.5f;
  static constexpr auto WhiteMaxOpacity = 0.45f;
  static constexpr auto MidPointMaxOpacity = 0.90f;
  static constexpr auto BlackMaxOpacity = 0.85f;

  using namespace DirectX;

  const auto [r, g, b, a] = color.GetRGBAFTuple();

  const auto rgbaVec = XMVectorSet(r, g, b, a);
  const auto hsvaVec = XMColorRGBToHSV(rgbaVec);
  const auto s = XMVectorGetY(hsvaVec);
  const auto v = XMVectorGetZ(hsvaVec);

  if (utility::almost_equal(v, MidPoint)) {
    return color.WithAlphaMultipliedBy(MidPointMaxOpacity);
  }

  const double lowerBound = (v > MidPoint) ? WhiteMaxOpacity : BlackMaxOpacity;
  const double maxDeviation = (v > MidPoint) ? (1.0f - MidPoint) : MidPoint;

  float maxSuppression = MidPointMaxOpacity - lowerBound;
  if (v > std::numeric_limits<float>::epsilon()) {
    maxSuppression = std::max(1 - (2 * s), 0.0f);
  }

  const auto deviation = std::abs(v - MidPoint) / maxDeviation;
  const float suppression = maxSuppression * deviation;
  return Color::Constant::FromRGBA128F(
    r, g, b, std::clamp(MidPointMaxOpacity - suppression, 0.0f, 1.0f));
}

constexpr auto NoiseWidth = 256;
constexpr auto NoiseHeight = 256;
auto CreateNoise() {
  alignas(uint32_t) std::array<uint32_t, NoiseWidth * NoiseHeight> ret;
  // Arbitrary seed
  std::minstd_rand generator(0xDEADBEEF);
  // We don't have a full 32-bits in this generator, so we'll
  // do two at a time and take the middle two bytes
  constexpr auto ChunkSize = 2;
  constexpr auto ChunkCount = ret.size() / ChunkSize;

  // sc_noiseOpacity from WinUI3 AcrylicBrush.h
  static constexpr auto AlphaF = 0.02f;
  static constexpr auto Alpha = static_cast<uint8_t>((AlphaF * 255.0f) + 0.5f);

  for (std::size_t i = 0; i < ChunkCount; ++i) {
    const auto it = ret.data() + (i * ChunkSize);
    const auto values = generator();
    static constexpr auto MakePixel
      = [](const uint32_t values, const std::size_t j) {
          // +1 to take middle two bytes
          const auto valueWithStraightAlpha = (values >> ((j + 1) * 8)) & 0xFF;
          // premultiplied alpha
          const auto value
            = static_cast<uint8_t>((valueWithStraightAlpha * AlphaF) + 0.5f);
          return std::bit_cast<uint32_t>(std::array<uint8_t, 4> {
            static_cast<uint8_t>(value),// r
            static_cast<uint8_t>(value),// g
            static_cast<uint8_t>(value),// b
            Alpha,
          });
        };
    it[0] = MakePixel(values, 0);
    it[1] = MakePixel(values, 1);
  }

  return ret;
}

}// namespace

class AcrylicController::Impl final {
 public:
  Impl(
    Kind,
    const CopySoftwareBitmap&,
    IUnknown* device,
    HWND,
    IDXGISwapChain*,
    const BasicSize<uint32_t>&,
    const Brush&);

  void Resize(uint32_t width, uint32_t height);

  void SetBrush(const Brush&);

 private:
  Kind mKind;
  CopySoftwareBitmap mCopySoftwareBitmap;
  HWND mHwnd;
  BasicSize<uint32_t> mSize;
  Brush mBrush;

  wil::com_ptr<WUC::ICompositor> mCompositor;
  wil::com_ptr<WUC::ICompositionGraphicsDevice> mDevice;
  wil::com_ptr<WUC::ICompositionTarget> mWindowTarget;
  wil::com_ptr<WUC::IVisual> mRootVisual;
  wil::com_ptr<WUC::IVisual> mSwapChainVisual;
  wil::com_ptr<WUC::IVisual> mBackdropVisual;

  void CreateAcrylicBackdrop(const AcrylicBrush& acrylicBrush);
  void CreateSolidColorBackdrop(const Color::Constant& color);
  void ApplyCurrentBrush();
};

AcrylicController::AcrylicController(
  const Kind kind,
  const CopySoftwareBitmap& paintDataCallback,
  IUnknown* device,
  HWND const hwnd,
  IDXGISwapChain* swapChain,
  const BasicSize<uint32_t>& size,
  const Brush& brush) {
  p.reset(
    new Impl(kind, paintDataCallback, device, hwnd, swapChain, size, brush));
}

AcrylicController::~AcrylicController() = default;

bool AcrylicController::IsSupported() noexcept {
  static bool ret {};
  static std::once_flag once;
  std::call_once(once, [&p = ret] {
    try {
      const auto compositor = wil::ActivateInstance<WUC::ICompositor>(
        RuntimeClass_Windows_UI_Composition_Compositor);
      p = static_cast<bool>(compositor);
    } catch (...) {
      p = false;
    }
  });
  return ret;
}

void AcrylicController::Resize(const uint32_t width, const uint32_t height) {
  p->Resize(width, height);
}

void AcrylicController::Impl::CreateAcrylicBackdrop(
  const AcrylicBrush& acrylicBrush) {
  const auto compositor2 = mCompositor.query<WUC::ICompositor2>();
  ///// Acrylic material - brush creation

  const auto rawTintColor = acrylicBrush.GetTintColor();
  const auto tintColor = WithTintModifier(rawTintColor);
  const auto luminosityColor
    = GetLuminosityColor(rawTintColor, acrylicBrush.GetLuminosityOpacity());

  wil::com_ptr<WUC::ICompositionBackdropBrush> backdropBrush;
  if (mKind == Kind::Desktop) {
    static constexpr BOOL enable = TRUE;
    if (SUCCEEDED(DwmSetWindowAttribute(
          mHwnd, DWMWA_USE_HOSTBACKDROPBRUSH, &enable, sizeof(BOOL)))) {
      CheckHResult(
        compositor2.query<WUC::ICompositor3>()->CreateHostBackdropBrush(
          backdropBrush.put()));
    }
  }
  if (!backdropBrush) {
    CheckHResult(compositor2->CreateBackdropBrush(backdropBrush.put()));
  }

  wil::com_ptr<WUC::ICompositionEffectBrush> blurEffectBrush;
  {
    const auto blurEffectSpec = Microsoft::WRL::Make<CompositionBlurEffect>();
    blurEffectSpec->SetBlurAmount(30.0f);
    blurEffectSpec->SetBorderMode(D2D1_BORDER_MODE_HARD);
    blurEffectSpec->CreateBrush(mCompositor.get(), blurEffectBrush.put());
  }

  wil::com_ptr<WUC::ICompositionEffectBrush> luminosityEffectBrush;
  {
    wil::com_ptr<WUC::ICompositionColorBrush> luminosityColorBrush;
    const auto [r, g, b, a] = luminosityColor.GetRGBA32Tuple();
    CheckHResult(mCompositor->CreateColorBrushWithColor(
      ABI::Windows::UI::Color {
        .A = a,
        .R = r,
        .G = g,
        .B = b,
      },
      luminosityColorBrush.put()));

    const auto luminosityEffectSpec
      = Microsoft::WRL::Make<CompositionBlendEffect>();
    luminosityEffectSpec->SetMode(D2D1_BLEND_MODE_LUMINOSITY);
    luminosityEffectSpec->CreateBrush(
      mCompositor.get(), luminosityEffectBrush.put());

    CheckHResult(luminosityEffectBrush->SetSourceParameter(
      MakeHSTRING(L"source").release(),
      luminosityColorBrush.query<WUC::ICompositionBrush>().get()));
  }

  wil::com_ptr<WUC::ICompositionEffectBrush> tintEffectBrush;
  {
    const auto [r, g, b, tintColorAlpha] = tintColor.GetRGBAFTuple();
    const auto a = acrylicBrush.GetTintOpacity() * tintColorAlpha;
    const auto keep = 1 - a;
    const auto tintEffectSpec
      = Microsoft::WRL::Make<CompositionColorMatrixEffect>();
    tintEffectSpec->SetColorMatrix(
      D2D1::Matrix5x4F({
        // clang-format off
        keep, 0, 0, 0,
        0, keep, 0, 0,
        0, 0, keep, 0,
        0, 0, 0, 1,
        r * a, g * a, b * a, 0,
        // clang-format on
      }));
    tintEffectSpec->CreateBrush(mCompositor.get(), tintEffectBrush.put());
  }

  wil::com_ptr<WUC::ICompositionEffectBrush> noiseEffectBrush;
  {
    wil::com_ptr<WUC::ICompositionDrawingSurface> wucSurface;
    using enum ABI::Windows::Graphics::DirectX::DirectXPixelFormat;
    using enum ABI::Windows::Graphics::DirectX::DirectXAlphaMode;
    CheckHResult(mDevice->CreateDrawingSurface(
      {NoiseWidth, NoiseHeight},
      DirectXPixelFormat_R8G8B8A8UIntNormalized,
      DirectXAlphaMode_Premultiplied,
      wucSurface.put()));

    {
      const auto interopSurface
        = wucSurface.query<WUC::ICompositionDrawingSurfaceInterop>();
      const auto noise = CreateNoise();
      POINT offset {};
      wil::com_ptr<IDXGISurface> dxgiSurface;
      CheckHResult(interopSurface->BeginDraw(
        nullptr, IID_PPV_ARGS(dxgiSurface.put()), &offset));
      mCopySoftwareBitmap(
        dxgiSurface.get(),
        {
          static_cast<uint32_t>(offset.x),
          static_cast<uint32_t>(offset.y),
        },
        noise.data(),
        {NoiseWidth, NoiseHeight},
        NoiseWidth * sizeof(decltype(noise)::value_type));
      CheckHResult(interopSurface->EndDraw());
    }

    wil::com_ptr<WUC::ICompositionSurfaceBrush> surfaceBrush;
    CheckHResult(mCompositor->CreateSurfaceBrush(surfaceBrush.put()));
    CheckHResult(surfaceBrush->put_Surface(
      wucSurface.query<WUC::ICompositionSurface>().get()));
    CheckHResult(surfaceBrush->put_Stretch(
      WUC::CompositionStretch::CompositionStretch_None));

    const auto borderEffectSpec
      = Microsoft::WRL::Make<CompositionBorderEffect>();
    borderEffectSpec->SetExtendX(D2D1_BORDER_EDGE_MODE_WRAP);
    borderEffectSpec->SetExtendY(D2D1_BORDER_EDGE_MODE_WRAP);
    const auto borderEffectBrush
      = borderEffectSpec->CreateBrush(mCompositor.get());
    CheckHResult(borderEffectBrush->SetSourceParameter(
      MakeHSTRING(L"source").release(),
      surfaceBrush.query<WUC::ICompositionBrush>().get()));

    const auto noiseEffectSpec
      = Microsoft::WRL::Make<CompositionCompositeEffect>();
    noiseEffectSpec->CreateBrush(mCompositor.get(), noiseEffectBrush.put());
    CheckHResult(noiseEffectBrush->SetSourceParameter(
      MakeHSTRING(L"source").release(),
      borderEffectBrush.query<WUC::ICompositionBrush>().get()));
  }

  ///// Acrylic: effect pipeline

  CheckHResult(blurEffectBrush->SetSourceParameter(
    MakeHSTRING(L"source").release(),
    backdropBrush.query<WUC::ICompositionBrush>().get()));

  CheckHResult(luminosityEffectBrush->SetSourceParameter(
    MakeHSTRING(L"destination").release(),
    blurEffectBrush.query<WUC::ICompositionBrush>().get()));

  CheckHResult(tintEffectBrush->SetSourceParameter(
    MakeHSTRING(L"source").release(),
    luminosityEffectBrush.query<WUC::ICompositionBrush>().get()));

  CheckHResult(noiseEffectBrush->SetSourceParameter(
    MakeHSTRING(L"destination").release(),
    tintEffectBrush.query<WUC::ICompositionBrush>().get()));

  const auto& lastEffect = noiseEffectBrush;

  ///// Acrylic: visual

  wil::com_ptr<WUC::ISpriteVisual> acrylicVisual;
  CheckHResult(mCompositor->CreateSpriteVisual(acrylicVisual.put()));
  CheckHResult(
    acrylicVisual->put_Brush(lastEffect.query<WUC::ICompositionBrush>().get()));
  mBackdropVisual = acrylicVisual.query<WUC::IVisual>();
  CheckHResult(mBackdropVisual->put_Size({
    static_cast<float>(mSize.mWidth),
    static_cast<float>(mSize.mHeight),
  }));
}

void AcrylicController::Impl::CreateSolidColorBackdrop(
  const Color::Constant& color) {
  const auto [r, g, b, a] = color.GetRGBA32Tuple();
  wil::com_ptr<WUC::ICompositionColorBrush> brush;
  CheckHResult(mCompositor->CreateColorBrushWithColor(
    ABI::Windows::UI::Color {
      .A = a,
      .R = r,
      .G = g,
      .B = b,
    },
    brush.put()));

  wil::com_ptr<WUC::ISpriteVisual> visual;
  CheckHResult(mCompositor->CreateSpriteVisual(visual.put()));
  CheckHResult(visual->put_Brush(brush.query<WUC::ICompositionBrush>().get()));
  mBackdropVisual = visual.query<WUC::IVisual>();
  CheckHResult(mBackdropVisual->put_Size({
    static_cast<float>(mSize.mWidth),
    static_cast<float>(mSize.mHeight),
  }));
}

AcrylicController::Impl::Impl(
  const Kind kind,
  const CopySoftwareBitmap& copyPixels,
  IUnknown* const device,
  HWND const hwnd,
  IDXGISwapChain* const swapChain,
  const BasicSize<uint32_t>& size,
  const Brush& brush)
  : mKind(kind),
    mCopySoftwareBitmap(copyPixels),
    mHwnd(hwnd),
    mSize(size),
    mBrush(brush) {
  mCompositor = wil::ActivateInstance<WUC::ICompositor>(
    RuntimeClass_Windows_UI_Composition_Compositor);
  const auto compositorInterop = mCompositor.query<WUC::ICompositorInterop>();
  CheckHResult(compositorInterop->CreateGraphicsDevice(device, mDevice.put()));
  const auto compositorDesktopInterop
    = mCompositor.query<WUC::Desktop::ICompositorDesktopInterop>();

  ///// application (swapchain)

  wil::com_ptr<WUC::ICompositionSurface> applicationSurface;
  CheckHResult(compositorInterop->CreateCompositionSurfaceForSwapChain(
    swapChain, applicationSurface.put()));

  wil::com_ptr<WUC::ICompositionSurfaceBrush> applicationSurfaceBrush;
  CheckHResult(mCompositor->CreateSurfaceBrush(applicationSurfaceBrush.put()));
  CheckHResult(applicationSurfaceBrush->put_Surface(applicationSurface.get()));
  CheckHResult(
    applicationSurfaceBrush->put_Stretch(WUC::CompositionStretch_None));
  CheckHResult(applicationSurfaceBrush->put_HorizontalAlignmentRatio(0.0f));
  CheckHResult(applicationSurfaceBrush->put_VerticalAlignmentRatio(0.0f));

  wil::com_ptr<WUC::ISpriteVisual> swapChainVisual;
  CheckHResult(mCompositor->CreateSpriteVisual(swapChainVisual.put()));
  swapChainVisual.query_to(mSwapChainVisual.put());
  CheckHResult(swapChainVisual->put_Brush(
    applicationSurfaceBrush.query<WUC::ICompositionBrush>().get()));
  CheckHResult(mSwapChainVisual->put_Size({
    static_cast<float>(size.mWidth),
    static_cast<float>(size.mHeight),
  }));

  ///// Root; visual tree is set up by SetBrush
  wil::com_ptr<WUC::IContainerVisual> rootVisual;
  CheckHResult(mCompositor->CreateContainerVisual(rootVisual.put()));
  rootVisual.query_to(mRootVisual.put());

  wil::com_ptr<WUC::Desktop::IDesktopWindowTarget> windowTarget;
  CheckHResult(compositorDesktopInterop->CreateDesktopWindowTarget(
    mHwnd, false, windowTarget.put()));
  windowTarget.query_to(mWindowTarget.put());
  CheckHResult(mWindowTarget->put_Root(mRootVisual.get()));

  this->ApplyCurrentBrush();
}

void AcrylicController::Impl::Resize(
  const uint32_t width,
  const uint32_t height) {
  if (width == mSize.mWidth && height == mSize.mHeight) {
    return;
  }
  mSize = {width, height};
  CheckHResult(mSwapChainVisual->put_Size({
    static_cast<float>(width),
    static_cast<float>(height),
  }));
  CheckHResult(mBackdropVisual->put_Size({
    static_cast<float>(width),
    static_cast<float>(height),
  }));
}

void AcrylicController::SetBrush(const Brush& brush) {
  p->SetBrush(brush);
}

void AcrylicController::Impl::SetBrush(const Brush& brush) {
  if (mBrush == brush) {
    return;
  }
  mBrush = brush;
  this->ApplyCurrentBrush();
}

void AcrylicController::Impl::ApplyCurrentBrush() {
  // As there's no PropertySet::InsertMatrix5x4(), we can't use animatable
  // properties we'll need to *at least* re-create the tint effect then tie the
  // acrylic visual effect stack back together.
  //
  // We may as well do the same for luminosity, even though we *could* animate
  // that property, given that:
  // - we need to modify the stack anyway
  // - system theme changes are rare, so having the value 'baked in' is probably
  //   better

  mBackdropVisual.reset();
  if (mBrush.IsAcrylicBrush()) {
    if (SystemSettings::Get().IsTransparencyEnabled()) {
      CreateAcrylicBackdrop(*mBrush.GetAcrylicBrush());
    } else {
      CreateSolidColorBackdrop(
        mBrush.GetAcrylicBrush()->GetFallbackColor().Resolve());
    }
  } else {
    CreateSolidColorBackdrop(mBrush.GetSolidColor()->Resolve());
  }

  wil::com_ptr<WUC::IVisualCollection> rootChildren;
  CheckHResult(mRootVisual.query<WUC::IContainerVisual>()->get_Children(
    rootChildren.put()));
  CheckHResult(rootChildren->RemoveAll());
  CheckHResult(rootChildren->InsertAtBottom(mBackdropVisual.get()));
  CheckHResult(rootChildren->InsertAtTop(mSwapChainVisual.get()));
}

}// namespace FredEmmott::GUI
