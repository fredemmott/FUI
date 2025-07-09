find_package(yoga CONFIG REQUIRED)

add_library(
  fredemmott-gui
  STATIC
  FredEmmott/GUI.hpp
  FredEmmott/GUI/ActivatedFlag.hpp
  FredEmmott/GUI/Brush.hpp
  FredEmmott/GUI/Color.hpp
  FredEmmott/GUI/EasingFunctions.hpp
  FredEmmott/GUI/ExitException.hpp
  FredEmmott/GUI/Font.cpp FredEmmott/GUI/Font.hpp
  FredEmmott/GUI/FontWeight.hpp
  FredEmmott/GUI/FrameRateRequirement.hpp
  FredEmmott/GUI/Immediate/Button.cpp FredEmmott/GUI/Immediate/Button.hpp
  FredEmmott/GUI/Immediate/Card.hpp
  FredEmmott/GUI/Immediate/CheckBox.cpp FredEmmott/GUI/Immediate/CheckBox.hpp
  FredEmmott/GUI/Immediate/ComboBox.cpp FredEmmott/GUI/Immediate/ComboBox.hpp
  FredEmmott/GUI/Immediate/ComboBoxButton.cpp FredEmmott/GUI/Immediate/ComboBoxButton.hpp
  FredEmmott/GUI/Immediate/ComboBoxItem.cpp FredEmmott/GUI/Immediate/ComboBoxItem.hpp
  FredEmmott/GUI/Immediate/ComboBoxPopup.cpp FredEmmott/GUI/Immediate/ComboBoxPopup.hpp
  FredEmmott/GUI/Immediate/ContentDialog.cpp FredEmmott/GUI/Immediate/ContentDialog.hpp
  FredEmmott/GUI/Immediate/Disabled.cpp FredEmmott/GUI/Immediate/Disabled.hpp
  FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.cpp FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp
  FredEmmott/GUI/Immediate/FontIcon.cpp FredEmmott/GUI/Immediate/FontIcon.hpp
  FredEmmott/GUI/Immediate/ID.hpp
  FredEmmott/GUI/Immediate/Label.cpp FredEmmott/GUI/Immediate/Label.hpp
  FredEmmott/GUI/Immediate/PopupWindow.cpp FredEmmott/GUI/Immediate/PopupWindow.hpp
  FredEmmott/GUI/Immediate/PushID.cpp FredEmmott/GUI/Immediate/PushID.hpp
  FredEmmott/GUI/Immediate/ResizeToFit.cpp FredEmmott/GUI/Immediate/ResizeToFit.hpp
  FredEmmott/GUI/Immediate/Result.hpp
  FredEmmott/GUI/Immediate/Root.cpp FredEmmott/GUI/Immediate/Root.hpp
  FredEmmott/GUI/Immediate/ScrollView.hpp
  FredEmmott/GUI/Immediate/StackPanel.hpp
  FredEmmott/GUI/Immediate/TextBlock.cpp FredEmmott/GUI/Immediate/TextBlock.hpp
  FredEmmott/GUI/Immediate/ToggleSwitch.cpp FredEmmott/GUI/Immediate/ToggleSwitch.hpp
  FredEmmott/GUI/Interpolation/CubicBezier.hpp
  FredEmmott/GUI/Interpolation/Linear.cpp FredEmmott/GUI/Interpolation/Linear.hpp
  FredEmmott/GUI/LinearGradientBrush.cpp FredEmmott/GUI/LinearGradientBrush.hpp
  FredEmmott/GUI/Orientation.hpp
  FredEmmott/GUI/Point.hpp
  FredEmmott/GUI/PseudoClasses.cpp FredEmmott/GUI/PseudoClasses.hpp
  FredEmmott/GUI/Rect.hpp
  FredEmmott/GUI/Renderer.hpp
  FredEmmott/GUI/Size.hpp
  FredEmmott/GUI/SolidColorBrush.hpp
  FredEmmott/GUI/StaticTheme.cpp FredEmmott/GUI/StaticTheme.hpp
  FredEmmott/GUI/StaticTheme/Resource.hpp
  FredEmmott/GUI/StaticTheme/Theme.hpp
  FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp
  FredEmmott/GUI/StaticTheme/detail/ResourceSupertype.hpp
  FredEmmott/GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp
  FredEmmott/GUI/StaticTheme/detail/Button.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/Button.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/ContentDialog.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/ContentDialog.handwritten.hpp
  FredEmmott/GUI/StaticTheme/detail/Generic.handwritten.cpp
  FredEmmott/GUI/StaticTheme/detail/Generic.handwritten.hpp
  FredEmmott/GUI/Style.cpp FredEmmott/GUI/Style.hpp
  FredEmmott/GUI/StyleClass.cpp FredEmmott/GUI/StyleClass.hpp
  FredEmmott/GUI/StyleProperty.hpp
  FredEmmott/GUI/StyleTransition.hpp
  FredEmmott/GUI/SystemFont.cpp FredEmmott/GUI/SystemFont.hpp
  FredEmmott/GUI/SystemSettings.cpp FredEmmott/GUI/SystemSettings.hpp
  FredEmmott/GUI/SystemTheme.cpp FredEmmott/GUI/SystemTheme.hpp
  FredEmmott/GUI/WidgetFont.cpp FredEmmott/GUI/WidgetFont.hpp
  FredEmmott/GUI/Widgets/Button.cpp FredEmmott/GUI/Widgets/Button.hpp
  FredEmmott/GUI/Widgets/Card.cpp FredEmmott/GUI/Widgets/Card.hpp
  FredEmmott/GUI/Widgets/CheckBox.cpp FredEmmott/GUI/Widgets/CheckBox.hpp
  FredEmmott/GUI/Widgets/Label.cpp FredEmmott/GUI/Widgets/Label.hpp
  FredEmmott/GUI/Widgets/PopupWindow.cpp FredEmmott/GUI/Widgets/PopupWindow.hpp
  FredEmmott/GUI/Widgets/RadioButton.cpp FredEmmott/GUI/Widgets/RadioButton.hpp
  FredEmmott/GUI/Widgets/ScrollBar.cpp FredEmmott/GUI/Widgets/ScrollBar.hpp
  FredEmmott/GUI/Widgets/ScrollBarButton.cpp FredEmmott/GUI/Widgets/ScrollBarButton.hpp
  FredEmmott/GUI/Widgets/ScrollBarThumb.cpp FredEmmott/GUI/Widgets/ScrollBarThumb.hpp
  FredEmmott/GUI/Widgets/ScrollView.cpp FredEmmott/GUI/Widgets/ScrollView.hpp
  FredEmmott/GUI/Widgets/StackPanel.cpp FredEmmott/GUI/Widgets/StackPanel.hpp
  FredEmmott/GUI/Widgets/TextBlock.cpp FredEmmott/GUI/Widgets/TextBlock.hpp
  FredEmmott/GUI/Widgets/ToggleSwitch.cpp FredEmmott/GUI/Widgets/ToggleSwitch.hpp
  FredEmmott/GUI/Widgets/ToggleSwitchKnob.cpp FredEmmott/GUI/Widgets/ToggleSwitchKnob.hpp
  FredEmmott/GUI/Widgets/ToggleSwitchThumb.cpp FredEmmott/GUI/Widgets/ToggleSwitchThumb.hpp
  FredEmmott/GUI/Widgets/Widget.cpp
  FredEmmott/GUI/Widgets/Widget.hpp
  FredEmmott/GUI/Widgets/WidgetList.hpp
  FredEmmott/GUI/Widgets/Widget_ComputeStyles.cpp
  FredEmmott/GUI/Widgets/Widget_StyleTransitions.cpp
  FredEmmott/GUI/Windows/Win32Window.cpp FredEmmott/GUI/Windows/Win32Window.hpp
  FredEmmott/GUI/Window.cpp FredEmmott/GUI/Window.hpp
  FredEmmott/GUI/assert.hpp
  FredEmmott/GUI/detail/font_detail.hpp
  FredEmmott/GUI/detail/immediate/Widget.hpp
  FredEmmott/GUI/detail/immediate/CaptionResultMixin.cpp FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp
  FredEmmott/GUI/detail/immediate/TextBlockStylesMixin.hpp
  FredEmmott/GUI/detail/immediate/ScopeableResultMixin.hpp
  FredEmmott/GUI/detail/immediate/StyledResultMixin.hpp
  FredEmmott/GUI/detail/immediate/ValueResultMixin.hpp
  FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp
  FredEmmott/GUI/detail/immediate/widget_from_result.hpp
  FredEmmott/GUI/detail/immediate_detail.cpp FredEmmott/GUI/detail/immediate_detail.hpp
  FredEmmott/GUI/detail/renderer_detail.cpp FredEmmott/GUI/detail/renderer_detail.hpp
  FredEmmott/GUI/detail/style_detail.hpp
  FredEmmott/GUI/detail/style/lazy_init_style.hpp
  FredEmmott/GUI/detail/system_font_detail.hpp
  FredEmmott/GUI/detail/widget_detail.hpp
  FredEmmott/GUI/detail/win32_detail.cpp FredEmmott/GUI/detail/win32_detail.hpp
  FredEmmott/GUI/detail/Widget/transitions.hpp
  FredEmmott/GUI/events/Event.hpp
  FredEmmott/GUI/events/MouseButton.hpp
  FredEmmott/GUI/events/MouseEvent.hpp
  FredEmmott/GUI/yoga.cpp FredEmmott/GUI/yoga.hpp
  FredEmmott/memory.hpp
  FredEmmott/memory/memory_detail.hpp
  FredEmmott/type_traits/concepts.hpp
  FredEmmott/utility/bitflag_enums.hpp
  FredEmmott/utility/moved_flag.hpp
  FredEmmott/utility/lazy_init.hpp
  FredEmmott/utility/type_tag.hpp
)
set(
  SKIA_SOURCES
  FredEmmott/GUI/Brush_Skia.cpp
  FredEmmott/GUI/LinearGradientBrush_Skia.cpp
  FredEmmott/GUI/SkiaRenderer.cpp FredEmmott/GUI/SkiaRenderer.hpp
  FredEmmott/GUI/SystemFont_Skia.cpp
  FredEmmott/GUI/Widgets/TextBlock_Skia.cpp
  FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.cpp FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.hpp
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

target_link_libraries(
  fredemmott-gui
  PUBLIC
  fredemmott-gui-config
  winui3-themes
  # vpckg
  yoga::yogacore
)
set(WINDOWS_SDK_LIBRARIES Dcomp Dwmapi User32 runtimeobject)
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

foreach (NAME IN LISTS WINDOWS_SDK_LIBRARIES)
  find_library("${NAME}_PATH" "${NAME}" REQUIRED)
  target_link_libraries(fredemmott-gui PRIVATE "${${NAME}_PATH}")
endforeach ()

get_target_property(HEADERS fredemmott-gui SOURCES)
list(FILTER HEADERS INCLUDE REGEX "\\.hpp$")

if (ENABLE_DEVELOPER_OPTIONS)
  # Explicit listing is needed for CMake to fully work correctly - but we can at least do a safety check
  file(GLOB_RECURSE GLOBBED_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "FredEmmott/*.hpp")
  foreach (HEADER IN LISTS GLOBBED_HEADERS)
    if (NOT HEADER IN_LIST HEADERS AND NOT HEADER IN_LIST SKIA_SOURCES AND NOT HEADER IN_LIST DIRECT2D_SOURCES)
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