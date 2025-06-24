find_package(yoga CONFIG REQUIRED)

add_library(
  fredemmott-gui
  STATIC
  GUI.hpp
  GUI/ActivatedFlag.hpp
  GUI/Brush.hpp
  GUI/Color.hpp
  GUI/EasingFunctions.hpp
  GUI/Font.cpp GUI/Font.hpp
  GUI/FrameRateRequirement.hpp
  GUI/Immediate/Button.cpp GUI/Immediate/Button.hpp
  GUI/Immediate/Card.hpp
  GUI/Immediate/ComboBox.cpp GUI/Immediate/ComboBox.hpp
  GUI/Immediate/ComboBoxButton.cpp GUI/Immediate/ComboBoxButton.hpp
  GUI/Immediate/ComboBoxItem.cpp GUI/Immediate/ComboBoxItem.hpp
  GUI/Immediate/ComboBoxPopup.cpp GUI/Immediate/ComboBoxPopup.hpp
  GUI/Immediate/Disabled.cpp GUI/Immediate/Disabled.hpp
  GUI/Immediate/EnqueueAdditionalFrame.cpp GUI/Immediate/EnqueueAdditionalFrame.hpp
  GUI/Immediate/FontIcon.cpp GUI/Immediate/FontIcon.hpp
  GUI/Immediate/ID.hpp
  GUI/Immediate/Label.cpp GUI/Immediate/Label.hpp
  GUI/Immediate/PopupWindow.cpp GUI/Immediate/PopupWindow.hpp
  GUI/Immediate/Root.cpp GUI/Immediate/Root.hpp
  GUI/Immediate/StackPanel.hpp
  GUI/Immediate/TextBlock.cpp GUI/Immediate/TextBlock.hpp
  GUI/Immediate/ToggleSwitch.cpp GUI/Immediate/ToggleSwitch.hpp
  GUI/Interpolation/CubicBezier.hpp
  GUI/Interpolation/Linear.cpp GUI/Interpolation/Linear.hpp
  GUI/LinearGradientBrush.cpp GUI/LinearGradientBrush.hpp
  GUI/Orientation.hpp
  GUI/Point.hpp
  GUI/PseudoClasses.cpp GUI/PseudoClasses.hpp
  GUI/Rect.hpp
  GUI/Size.hpp
  GUI/SolidColorBrush.hpp
  GUI/StaticTheme.cpp GUI/StaticTheme.hpp
  GUI/StaticTheme/Resource.hpp
  GUI/StaticTheme/Theme.hpp
  GUI/StaticTheme/detail/ResolveColor.hpp
  GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp
  GUI/Style.cpp GUI/Style.hpp
  GUI/StyleClass.cpp GUI/StyleClass.hpp
  GUI/StyleProperty.hpp>
  GUI/StyleTransition.hpp
  GUI/SystemFont.hpp
  GUI/SystemSettings.cpp GUI/SystemSettings.hpp
  GUI/SystemTheme.cpp GUI/SystemTheme.hpp
  GUI/WidgetFont.cpp GUI/WidgetFont.hpp
  GUI/Widgets/Button.cpp GUI/Widgets/Button.hpp
  GUI/Widgets/Card.cpp GUI/Widgets/Card.hpp
  GUI/Widgets/Label.cpp GUI/Widgets/Label.hpp
  GUI/Widgets/PopupWIndow.cpp GUI/Widgets/PopupWindow.hpp
  GUI/Widgets/ScrollBar.cpp GUI/Widgets/ScrollBar.hpp
  GUI/Widgets/ScrollBarButton.cpp GUI/Widgets/ScrollBarButton.hpp
  GUI/Widgets/ScrollBarThumb.cpp GUI/Widgets/ScrollBarThumb.hpp
  GUI/Widgets/ScrollView.cpp GUI/Widgets/ScrollView.hpp
  GUI/Widgets/StackPanel.cpp GUI/Widgets/StackPanel.hpp
  GUI/Widgets/TextBlock.cpp GUI/Widgets/TextBlock.hpp
  GUI/Widgets/ToggleSwitch.cpp GUI/Widgets/ToggleSwitch.hpp
  GUI/Widgets/ToggleSwitchKnob.cpp GUI/Widgets/ToggleSwitchKnob.hpp
  GUI/Widgets/ToggleSwitchThumb.cpp GUI/Widgets/ToggleSwitchThumb.hpp
  GUI/Widgets/Widget.cpp
  GUI/Widgets/Widget.hpp
  GUI/Widgets/Widget_ComputeStyles.cpp
  GUI/Widgets/Widget_StyleTransitions.cpp
  GUI/Windows/Win32Direct3D12GaneshWindow.cpp GUI/Windows/Win32Direct3D12GaneshWindow.hpp
  GUI/detail/font_detail.hpp
  GUI/detail/immediate/Widget.hpp
  GUI/detail/immediate_detail.cpp GUI/detail/immediate_detail.hpp
  GUI/detail/style_detail.hpp
  GUI/detail/widget_detail.hpp
  GUI/events/Event.hpp
  GUI/events/MouseButton.hpp
  GUI/events/MouseEvent.hpp
  GUI/yoga.cpp GUI/yoga.hpp
  memory.hpp
  memory/memory_detail.hpp
  type_traits/concepts.hpp
  utility/bitflag_enums.hpp
  utility/lazy_init.hpp
  utility/type_tag.hpp
)

target_link_libraries(
  fredemmott-gui
  PUBLIC
  fredemmott-gui-config
  # vpckg
  yoga::yogacore
  PRIVATE
  winui3-themes
  # system
  Dcomp
  Dwmapi
  User32
)
target_compile_definitions(
  fredemmott-gui
  PUBLIC
  UNICODE=1
  _UNICODE=1
)
target_include_directories(
  fredemmott-gui
  PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

if(ENABLE_SKIA)
  target_sources(
    fredemmott-gui
    PRIVATE
    GUI/Brush_Skia.cpp
    GUI/LinearGradientBrush_Skia.cpp
    GUI/SystemFont_Skia.cpp
  )
  target_link_libraries(
    fredemmott-gui
    PUBLIC
    skia
    PRIVATE
    unofficial::skia::modules::skunicode_icu
    unofficial::skia::modules::skparagraph
  )
endif()
