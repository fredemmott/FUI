find_package(felly CONFIG REQUIRED)

add_library(
  fredemmott-gui
  STATIC
  FredEmmott/GUI.hpp
  FredEmmott/GUI/AcrylicBrush.cpp
  FredEmmott/GUI/AcrylicBrush.hpp
  FredEmmott/GUI/Brush.cpp
  FredEmmott/GUI/Brush.hpp
  FredEmmott/GUI/Color.hpp
  FredEmmott/GUI/CornerRadius.hpp
  FredEmmott/GUI/EasingFunctions.hpp
  FredEmmott/GUI/Edges.hpp
  FredEmmott/GUI/ExitException.hpp
  FredEmmott/GUI/FocusManager.cpp FredEmmott/GUI/FocusManager.hpp
  FredEmmott/GUI/Font.hpp
  FredEmmott/GUI/FontWeight.hpp
  FredEmmott/GUI/FrameRateRequirement.hpp
  FredEmmott/GUI/IconProvider.hpp
  FredEmmott/GUI/Immediate/Button.cpp FredEmmott/GUI/Immediate/Button.hpp
  FredEmmott/GUI/Immediate/Card.hpp
  FredEmmott/GUI/Immediate/CheckBox.cpp FredEmmott/GUI/Immediate/CheckBox.hpp
  FredEmmott/GUI/Immediate/ComboBox.cpp FredEmmott/GUI/Immediate/ComboBox.hpp
  FredEmmott/GUI/Immediate/ComboBoxButton.cpp FredEmmott/GUI/Immediate/ComboBoxButton.hpp
  FredEmmott/GUI/Immediate/ComboBoxItem.cpp FredEmmott/GUI/Immediate/ComboBoxItem.hpp
  FredEmmott/GUI/Immediate/ComboBoxPopup.cpp FredEmmott/GUI/Immediate/ComboBoxPopup.hpp
  FredEmmott/GUI/Immediate/ContentDialog.cpp FredEmmott/GUI/Immediate/ContentDialog.hpp
  FredEmmott/GUI/Immediate/Disabled.cpp FredEmmott/GUI/Immediate/Disabled.hpp
  FredEmmott/GUI/Immediate/FontIcon.cpp FredEmmott/GUI/Immediate/FontIcon.hpp
  FredEmmott/GUI/Immediate/ID.hpp
  FredEmmott/GUI/Immediate/HyperlinkButton.cpp FredEmmott/GUI/Immediate/HyperlinkButton.hpp
  FredEmmott/GUI/Immediate/Label.cpp FredEmmott/GUI/Immediate/Label.hpp
  FredEmmott/GUI/Immediate/MenuFlyout.cpp
  FredEmmott/GUI/Immediate/MenuFlyout.hpp
  FredEmmott/GUI/Immediate/NavigationView.hpp
  FredEmmott/GUI/Immediate/NumberBox.hpp
  FredEmmott/GUI/Immediate/PopupWindow.cpp FredEmmott/GUI/Immediate/PopupWindow.hpp
  FredEmmott/GUI/Immediate/ProgressRing.hpp
  FredEmmott/GUI/Immediate/PushID.cpp FredEmmott/GUI/Immediate/PushID.hpp
  FredEmmott/GUI/Immediate/RadioButton.hpp
  FredEmmott/GUI/Immediate/RadioButtons.cpp FredEmmott/GUI/Immediate/RadioButtons.hpp
  FredEmmott/GUI/Immediate/ResizeToFit.cpp FredEmmott/GUI/Immediate/ResizeToFit.hpp
  FredEmmott/GUI/Immediate/Result.hpp
  FredEmmott/GUI/Immediate/Root.cpp FredEmmott/GUI/Immediate/Root.hpp
  FredEmmott/GUI/Immediate/ScrollView.hpp
  FredEmmott/GUI/Immediate/Slider.cpp
  FredEmmott/GUI/Immediate/Slider.hpp
  FredEmmott/GUI/Immediate/StackPanel.hpp
  FredEmmott/GUI/Immediate/TextBlock.cpp FredEmmott/GUI/Immediate/TextBlock.hpp
  FredEmmott/GUI/Immediate/TextBox.cpp
  FredEmmott/GUI/Immediate/TextBox.hpp
  FredEmmott/GUI/Immediate/TitleBar.cpp
  FredEmmott/GUI/Immediate/TitleBar.hpp
  FredEmmott/GUI/Immediate/ToggleSwitch.cpp FredEmmott/GUI/Immediate/ToggleSwitch.hpp
  FredEmmott/GUI/Immediate/ToolTip.cpp
  FredEmmott/GUI/Immediate/ToolTip.hpp
  FredEmmott/GUI/Immediate/WasActivated.cpp
  FredEmmott/GUI/Immediate/WasActivated.hpp
  FredEmmott/GUI/Immediate/selectable_key.hpp
  FredEmmott/GUI/Interpolation/CubicBezier.hpp
  FredEmmott/GUI/Interpolation/Linear.cpp FredEmmott/GUI/Interpolation/Linear.hpp
  FredEmmott/GUI/LinearGradientBrush.cpp FredEmmott/GUI/LinearGradientBrush.hpp
  FredEmmott/GUI/Orientation.hpp
  FredEmmott/GUI/NativeWaitable.hpp
  FredEmmott/GUI/Point.hpp
  FredEmmott/GUI/PseudoClasses.cpp FredEmmott/GUI/PseudoClasses.hpp
  FredEmmott/GUI/Rect.hpp
  FredEmmott/GUI/Renderer.hpp
  FredEmmott/GUI/Size.hpp
  FredEmmott/GUI/SoftwareBitmap.hpp
  FredEmmott/GUI/SolidColorBrush.hpp
  FredEmmott/GUI/StaticTheme.hpp
  FredEmmott/GUI/StaticTheme/Resource.hpp
  FredEmmott/GUI/StaticTheme/Theme.hpp
  FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp
  FredEmmott/GUI/StaticTheme/detail/ResourceSupertype.hpp
  FredEmmott/GUI/StaticTheme/detail/StaticThemedAcrylicBrush.hpp
  FredEmmott/GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp
  FredEmmott/GUI/StaticTheme/detail/Button.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/Button.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/ContentDialog.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/ContentDialog.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/Generic.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/Generic.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/HyperlinkButton.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/HyperlinkButton.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/MenuFlyout.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/MenuFlyout.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/NavigationView.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/NavigationView.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/TextBox.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/TextBox.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/TitleBar.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/TitleBar.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/ToolTip.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/ToolTip.handwritten.hpp
  FredEmmott/GUI/Style.cpp FredEmmott/GUI/Style.hpp
  FredEmmott/GUI/StyleClass.cpp FredEmmott/GUI/StyleClass.hpp
  FredEmmott/GUI/StyleProperty.hpp
  FredEmmott/GUI/StylePropertyTypes.hpp
  FredEmmott/GUI/StyleTransition.hpp
  FredEmmott/GUI/SystemFont.cpp FredEmmott/GUI/SystemFont.hpp
  FredEmmott/GUI/SystemSettings.hpp
  FredEmmott/GUI/SystemTheme.hpp
  FredEmmott/GUI/WidgetFont.cpp FredEmmott/GUI/WidgetFont.hpp
  FredEmmott/GUI/Widgets/Button.cpp FredEmmott/GUI/Widgets/Button.hpp
  FredEmmott/GUI/Widgets/Card.cpp FredEmmott/GUI/Widgets/Card.hpp
  FredEmmott/GUI/Widgets/CheckBox.cpp FredEmmott/GUI/Widgets/CheckBox.hpp
  FredEmmott/GUI/Widgets/ComboBoxItem.cpp
  FredEmmott/GUI/Widgets/ComboBoxItem.hpp
  FredEmmott/GUI/Widgets/Focusable.hpp
  FredEmmott/GUI/Widgets/HyperlinkButton.cpp FredEmmott/GUI/Widgets/HyperlinkButton.hpp
  FredEmmott/GUI/Widgets/Label.cpp
  FredEmmott/GUI/Widgets/Label.hpp
  FredEmmott/GUI/Widgets/MenuFlyoutItem.cpp
  FredEmmott/GUI/Widgets/MenuFlyoutItem.hpp
  FredEmmott/GUI/Widgets/NavigationView.cpp
  FredEmmott/GUI/Widgets/NavigationView.hpp
  FredEmmott/GUI/Widgets/NavigationViewItem.cpp
  FredEmmott/GUI/Widgets/NavigationViewItem.hpp
  FredEmmott/GUI/Widgets/NavigationViewSettingsItem.cpp
  FredEmmott/GUI/Widgets/NavigationViewSettingsItem.hpp
  FredEmmott/GUI/Widgets/NavigationViewButton.cpp
  FredEmmott/GUI/Widgets/NavigationViewButton.hpp
  FredEmmott/GUI/Widgets/NavigationViewBackButton.cpp
  FredEmmott/GUI/Widgets/NavigationViewBackButton.hpp
  FredEmmott/GUI/Widgets/NavigationViewTogglePaneButton.cpp
  FredEmmott/GUI/Widgets/NavigationViewTogglePaneButton.hpp
  FredEmmott/GUI/Widgets/PopupWindow.cpp FredEmmott/GUI/Widgets/PopupWindow.hpp
  FredEmmott/GUI/Widgets/ProgressRing.cpp
  FredEmmott/GUI/Widgets/ProgressRing.hpp
  FredEmmott/GUI/Widgets/RadioButton.cpp FredEmmott/GUI/Widgets/RadioButton.hpp
  FredEmmott/GUI/Widgets/ScrollBar.cpp FredEmmott/GUI/Widgets/ScrollBar.hpp
  FredEmmott/GUI/Widgets/ScrollBarButton.cpp FredEmmott/GUI/Widgets/ScrollBarButton.hpp
  FredEmmott/GUI/Widgets/ScrollBarThumb.cpp FredEmmott/GUI/Widgets/ScrollBarThumb.hpp
  FredEmmott/GUI/Widgets/ScrollView.cpp FredEmmott/GUI/Widgets/ScrollView.hpp
  FredEmmott/GUI/Widgets/Slider.cpp FredEmmott/GUI/Widgets/Slider.hpp
  FredEmmott/GUI/Widgets/StackPanel.cpp FredEmmott/GUI/Widgets/StackPanel.hpp
  FredEmmott/GUI/Widgets/TextBlock.cpp FredEmmott/GUI/Widgets/TextBlock.hpp
  FredEmmott/GUI/Widgets/TextBox.hpp
  FredEmmott/GUI/Widgets/TitleBar.cpp
  FredEmmott/GUI/Widgets/TitleBar.hpp
  FredEmmott/GUI/Widgets/ToggleSwitch.cpp FredEmmott/GUI/Widgets/ToggleSwitch.hpp
  FredEmmott/GUI/Widgets/ToggleSwitchKnob.cpp FredEmmott/GUI/Widgets/ToggleSwitchKnob.hpp
  FredEmmott/GUI/Widgets/Widget.cpp
  FredEmmott/GUI/Widgets/Widget.hpp
  FredEmmott/GUI/Widgets/WidgetList.hpp
  FredEmmott/GUI/Widgets/Widget_ComputeStyles.cpp
  FredEmmott/GUI/Widgets/Widget_StyleTransitions.cpp
  FredEmmott/GUI/Window.cpp
  FredEmmott/GUI/Window.hpp
  FredEmmott/GUI/WindowBackdrop.hpp
  FredEmmott/GUI/assert.hpp
  FredEmmott/GUI/detail/AutomationActivityFlag.hpp
  FredEmmott/GUI/detail/SelectionPill.cpp
  FredEmmott/GUI/detail/SelectionPill.hpp
  FredEmmott/GUI/detail/font_detail.hpp
  FredEmmott/GUI/detail/icu.hpp
  FredEmmott/GUI/detail/immediate/CaptionResultMixin.cpp
  FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp
  FredEmmott/GUI/detail/immediate/ScopeableResultMixin.hpp
  FredEmmott/GUI/detail/immediate/SelectionManager.hpp
  FredEmmott/GUI/detail/immediate/StyledResultMixin.hpp
  FredEmmott/GUI/detail/immediate/TextBlockStylesResultMixin.hpp
  FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp
  FredEmmott/GUI/detail/immediate/ValueResultMixin.hpp
  FredEmmott/GUI/detail/immediate/Widget.cpp
  FredEmmott/GUI/detail/immediate/Widget.hpp
  FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp
  FredEmmott/GUI/detail/immediate/widget_from_result.hpp
  FredEmmott/GUI/detail/immediate_detail.cpp FredEmmott/GUI/detail/immediate_detail.hpp
  FredEmmott/GUI/detail/renderer_detail.cpp FredEmmott/GUI/detail/renderer_detail.hpp
  FredEmmott/GUI/detail/skia_paragraph.cpp FredEmmott/GUI/detail/skia_paragraph.hpp
  FredEmmott/GUI/detail/style_detail.hpp
  FredEmmott/GUI/detail/system_font_detail.hpp
  FredEmmott/GUI/detail/widget_detail.hpp
  FredEmmott/GUI/detail/Widget/ScrollBar.hpp
  FredEmmott/GUI/detail/Widget/transitions.hpp
  FredEmmott/GUI/events/Event.hpp
  FredEmmott/GUI/events/HitTestEvent.hpp
  FredEmmott/GUI/events/KeyCode.hpp
  FredEmmott/GUI/events/KeyEvent.hpp
  FredEmmott/GUI/events/MouseButton.hpp
  FredEmmott/GUI/events/MouseEvent.hpp
  FredEmmott/GUI/events/TextInputEvent.hpp
  FredEmmott/GUI/yoga.cpp FredEmmott/GUI/yoga.hpp
  FredEmmott/type_traits/concepts.hpp
  FredEmmott/utility/almost_equal.hpp
  FredEmmott/utility/bitflag_enums.hpp
  FredEmmott/utility/drop_last_t.hpp
  FredEmmott/utility/unordered_map.hpp
)
set(
  SKIA_SOURCES
  FredEmmott/GUI/Brush_Skia.cpp
  FredEmmott/GUI/LinearGradientBrush_Skia.cpp
  FredEmmott/GUI/SkiaRenderer.cpp FredEmmott/GUI/SkiaRenderer.hpp
  FredEmmott/GUI/SystemFont_Skia.cpp
  FredEmmott/GUI/Widgets/TextBlock_Skia.cpp
)
# Skia sources that are platform-specific — one of these pairs is selected
# based on WIN32.
set(
  SKIA_WIN32_SOURCES
  FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.cpp
  FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.hpp
)
set(
  SKIA_SDL_SOURCES
  FredEmmott/GUI/Sdl/SdlSkiaVulkanWindow.cpp
  FredEmmott/GUI/Sdl/SdlSkiaVulkanWindow.hpp
)
set(
  DIRECT2D_SOURCES
  FredEmmott/GUI/Windows/Win32Direct2DWindow.cpp FredEmmott/GUI/Windows/Win32Direct2DWindow.hpp
  FredEmmott/GUI/Direct2DRenderer.cpp FredEmmott/GUI/Direct2DRenderer.hpp
  FredEmmott/GUI/LinearGradientBrush_Direct2D.cpp
  FredEmmott/GUI/SystemFont_DirectWrite.cpp
  FredEmmott/GUI/detail/direct_write_detail/DirectWriteFontProvider.cpp FredEmmott/GUI/detail/direct_write_detail/DirectWriteFontProvider.hpp
  FredEmmott/GUI/Brush_Direct2D.cpp
  FredEmmott/GUI/Widgets/TextBlock_DirectWrite.cpp
)

# Files only built on Windows; added to fredemmott-gui via target_sources
# in the if (WIN32) branch below. Linux replacements live in
# FredEmmott/GUI/Linux/.
set(
  WIN32_ONLY_SOURCES
  # System*.cpp / IconProvider.cpp / StaticTheme.cpp / Font.cpp have
  # Win32-only bodies (SPI_*, GetSysColor, wil::com_ptr<IUISettings3>,
  # ExtractIconW, win32_detail::CheckHResult). Linux replacements live
  # under FredEmmott/GUI/Linux/.
  FredEmmott/GUI/Font.cpp
  FredEmmott/GUI/IconProvider.cpp
  FredEmmott/GUI/StaticTheme.cpp
  FredEmmott/GUI/SystemSettings.cpp
  FredEmmott/GUI/SystemTheme.cpp
  # NumberBox.cpp assumes sizeof(wchar_t) == sizeof(UChar) (i.e., 16 bit).
  # True on MSVC/Win32, false on Linux (wchar_t is 32-bit). Linux uses
  # FredEmmott/GUI/Linux/NumberBox.cpp instead.
  FredEmmott/GUI/Immediate/NumberBox.cpp
  # TextBox.cpp is built around the Win32 TSF IME stack (TSFTextStore,
  # CheckHResult, etc.). Gating inline would be invasive; the Linux IME
  # impl uses SDL3 text-input events. Only the .cpp is excluded; the
  # header stays visible so cross-platform widgets that reference
  # TextBox types still compile.
  FredEmmott/GUI/Widgets/TextBox.cpp
  # SwapChain / GPUTexture / SwapChainPanel — Win32-shared-HANDLE IPC;
  # Linux replacement uses dmabuf-fd in the §3.3 architectural pass.
  FredEmmott/GUI/SwapChain.cpp
  FredEmmott/GUI/SwapChain.hpp
  FredEmmott/GUI/SwapChain_Resources.hpp
  FredEmmott/GUI/Immediate/GPUTexture.cpp
  FredEmmott/GUI/Immediate/GPUTexture.hpp
  FredEmmott/GUI/Immediate/SwapChainPanel.hpp
  FredEmmott/GUI/Widgets/GPUTexture.cpp
  FredEmmott/GUI/Widgets/GPUTexture.hpp
  FredEmmott/GUI/Widgets/SwapChainPanel.cpp
  FredEmmott/GUI/Widgets/SwapChainPanel.hpp
  # Windowing / compositor backdrops — Win32-only by nature.
  FredEmmott/GUI/Windows/AcrylicController.cpp
  FredEmmott/GUI/Windows/AcrylicController.hpp
  FredEmmott/GUI/Windows/DirectCompositionController.cpp
  FredEmmott/GUI/Windows/DirectCompositionController.hpp
  FredEmmott/GUI/Windows/MicaController.cpp
  FredEmmott/GUI/Windows/MicaController.hpp
  FredEmmott/GUI/Windows/Win32Window.cpp
  FredEmmott/GUI/Windows/Win32Window.hpp
  # HRESULT / IME (TSF) / UI Automation helpers — all Win32.
  FredEmmott/GUI/detail/win32_detail.cpp
  FredEmmott/GUI/detail/win32_detail.hpp
  FredEmmott/GUI/detail/win32_detail/COMImplementation.hpp
  FredEmmott/GUI/detail/win32_detail/CopySoftwareBitmap.cpp
  FredEmmott/GUI/detail/win32_detail/CopySoftwareBitmap.hpp
  FredEmmott/GUI/detail/win32_detail/TSFTextStore.cpp
  FredEmmott/GUI/detail/win32_detail/TSFTextStore.hpp
  FredEmmott/GUI/detail/win32_detail/UIANode.cpp
  FredEmmott/GUI/detail/win32_detail/UIANode.hpp
  FredEmmott/GUI/detail/win32_detail/UIARoot.cpp
  FredEmmott/GUI/detail/win32_detail/UIARoot.hpp
)

# SDL3-backed windowing base. Cross-platform-eligible (Linux today; macOS
# and an SDL3-on-Windows portability path are planned). Pulls SDL3::SDL3
# via vcpkg sdl3 port.
set(
  SDL_SOURCES
  FredEmmott/GUI/Sdl/SdlWindow.cpp
  FredEmmott/GUI/Sdl/SdlWindow.hpp
  # Generic SDL-layer interface for popup mouse-passthrough; per-platform
  # implementation lives outside Sdl/ (Linux supplies it via
  # Linux/TooltipPassthrough.cpp).
  FredEmmott/GUI/detail/sdl_detail/PopupInputPassthrough.hpp
)

# Linux replacements for the Win32-only bodies above. SDL3 powers
# windowing/input/IME; Skia-on-Vulkan handles rendering.
set(
  LINUX_ONLY_SOURCES
  FredEmmott/GUI/Linux/Font.cpp
  FredEmmott/GUI/Linux/IconProvider.cpp
  FredEmmott/GUI/Linux/NumberBox.cpp
  FredEmmott/GUI/Linux/StaticTheme.cpp
  FredEmmott/GUI/Linux/SystemSettings.cpp
  FredEmmott/GUI/Linux/SystemTheme.cpp
  FredEmmott/GUI/Linux/TextBox.cpp
  # Linux implementation of sdl_detail::{MakePopupInputPassthrough,
  # RestakeTooltipInputRegion} (Wayland + X11). Isolated TU because X11
  # headers pollute the global namespace with KeyCode/Window/etc.
  FredEmmott/GUI/Linux/TooltipPassthrough.cpp
)

if (WIN32)
  target_sources(fredemmott-gui PRIVATE ${WIN32_ONLY_SOURCES})
endif ()

if (LINUX)
  target_sources(fredemmott-gui PRIVATE ${SDL_SOURCES} ${LINUX_ONLY_SOURCES})

  # SDL3 powers windowing / input / clipboard on Linux.
  find_package(SDL3 CONFIG REQUIRED)
  target_link_libraries(fredemmott-gui PUBLIC SDL3::SDL3)

  # Wayland + X11 used directly by TooltipPassthrough.cpp to set an empty
  # input region on tooltip popups (Win32's WS_EX_TRANSPARENT analogue).
  # SDL3 already pulls these in transitively at runtime; we just need the
  # headers and explicit link for our own calls.
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
  find_package(X11 REQUIRED COMPONENTS Xext)
  target_link_libraries(
    fredemmott-gui
    PRIVATE
    PkgConfig::WAYLAND_CLIENT
    X11::X11
    X11::Xext
  )
endif ()

if (APPLE)
  # macOS not yet implemented.
endif ()

find_package(Boost CONFIG REQUIRED COMPONENTS container)
find_package(yoga CONFIG REQUIRED)

target_link_libraries(
  fredemmott-gui
  PUBLIC
  fredemmott-gui-config
  winui3-themes
  # vpckg
  felly::felly
  yoga::yogacore
  Boost::container
)
set(WINDOWS_SDK_LIBRARIES Comctl32 dxguid Dcomp Dwmapi User32 runtimeobject Uiautomationcore CoreMessaging)
target_compile_definitions(
  fredemmott-gui
  PUBLIC
  UNICODE=1
  _UNICODE=1
  PRIVATE
  NOMINMAX=1
)
target_include_directories(
  fredemmott-gui
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_features(
  fredemmott-gui
  PUBLIC
  cxx_std_23
)

if (ENABLE_SKIA)
  include(skia)
  target_sources(fredemmott-gui PRIVATE ${SKIA_SOURCES})
  if (WIN32)
    target_sources(fredemmott-gui PRIVATE ${SKIA_WIN32_SOURCES})
  else ()
    target_sources(fredemmott-gui PRIVATE ${SKIA_SDL_SOURCES})
    # SdlSkiaVulkanWindow calls the Vulkan loader directly; Skia already
    # uses vulkan-headers but vkCreateInstance & friends live
    # in libvulkan.
    find_package(Vulkan REQUIRED)
    target_link_libraries(fredemmott-gui PUBLIC Vulkan::Vulkan)
  endif ()
  target_link_libraries(
    fredemmott-gui
    PUBLIC
    skia
    PRIVATE
    unofficial::skia::modules::skunicode_icu
    unofficial::skia::modules::skparagraph
  )
endif ()

if (ENABLE_DIRECT2D)
  target_sources(fredemmott-gui PRIVATE ${DIRECT2D_SOURCES})
  list(
    APPEND
    WINDOWS_SDK_LIBRARIES
    DXGI
    D2d1
    Dwrite
    D3d11
  )
endif ()

if (ENABLE_ICU)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)
  target_link_libraries(fredemmott-gui PRIVATE PkgConfig::ICU)
elseif (WIN32)
  list(APPEND WINDOWS_SDK_LIBRARIES icuuc icuin)
endif ()

if (WIN32)
  foreach (NAME IN LISTS WINDOWS_SDK_LIBRARIES)
    find_library("${NAME}_PATH" "${NAME}" REQUIRED)
    target_link_libraries(fredemmott-gui PRIVATE "${${NAME}_PATH}")
  endforeach ()
endif ()

get_target_property(HEADERS fredemmott-gui SOURCES)
list(FILTER HEADERS INCLUDE REGEX "\\.hpp$")

if (ENABLE_DEVELOPER_OPTIONS)
  # Explicit listing is needed for CMake to fully work correctly - but we can at least do a safety check
  file(GLOB_RECURSE GLOBBED_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "FredEmmott/*.hpp")
  foreach (HEADER IN LISTS GLOBBED_HEADERS)
    if (
      NOT HEADER IN_LIST HEADERS
      AND NOT HEADER IN_LIST SKIA_SOURCES
      AND NOT HEADER IN_LIST SKIA_WIN32_SOURCES
      AND NOT HEADER IN_LIST SKIA_SDL_SOURCES
      AND NOT HEADER IN_LIST DIRECT2D_SOURCES
      AND NOT HEADER IN_LIST WIN32_ONLY_SOURCES
      AND NOT HEADER IN_LIST SDL_SOURCES
      AND NOT HEADER IN_LIST LINUX_ONLY_SOURCES
    )
      message(FATAL_ERROR "Header '${HEADER}' must be explicitly added to ${CMAKE_CURRENT_LIST_FILE}")
    endif ()
  endforeach ()
endif ()

target_sources(fredemmott-gui PUBLIC FILE_SET HEADERS FILES "${HEADERS}")
install(
  TARGETS fredemmott-gui
  EXPORT exports
  ARCHIVE FILE_SET HEADERS
)
configure_file(
  "exports-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/exports-config.cmake"
  @ONLY
  NEWLINE_STYLE LF
)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/exports-config.cmake"
  RENAME "fredemmott-gui-config.cmake"
  DESTINATION lib/cmake/fredemmott-gui
)

install(
  EXPORT
  exports
  NAMESPACE fredemmott-gui::
  FILE fredemmott-gui-targets.cmake
  DESTINATION lib/cmake/fredemmott-gui
)

# For projects pulling this in via add_subdirectory()
add_library(fredemmott-gui::fredemmott-gui ALIAS fredemmott-gui)
