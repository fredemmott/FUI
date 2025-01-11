find_package(yoga CONFIG REQUIRED)
include(skia)

add_subdirectory(xaml-to-fui-statictheme)

add_executable(
  HelloSkia-Win32-Ganesh-D3D12
  WIN32
  app.exe.manifest
  main.cpp

  FredEmmott/GUI.hpp
  FredEmmott/GUI/ActivatedFlag.hpp
  FredEmmott/GUI/Brush.hpp
  FredEmmott/GUI/Color.hpp
  FredEmmott/GUI/EasingFunctions.hpp
  FredEmmott/GUI/Font.cpp FredEmmott/GUI/Font.hpp
  FredEmmott/GUI/FrameRateRequirement.hpp
  FredEmmott/GUI/Immediate/Button.cpp FredEmmott/GUI/Immediate/Button.hpp
  FredEmmott/GUI/Immediate/ComboBox.cpp FredEmmott/GUI/Immediate/ComboBox.hpp
  FredEmmott/GUI/Immediate/ComboBoxButton.cpp FredEmmott/GUI/Immediate/ComboBoxButton.hpp
  FredEmmott/GUI/Immediate/ComboBoxPopup.cpp FredEmmott/GUI/Immediate/ComboBoxPopup.hpp
  FredEmmott/GUI/Immediate/ComboBoxItem.cpp FredEmmott/GUI/Immediate/ComboBoxItem.hpp
  FredEmmott/GUI/Immediate/Card.hpp
  FredEmmott/GUI/Immediate/Disabled.cpp FredEmmott/GUI/Immediate/Disabled.hpp
  FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.cpp FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp
  FredEmmott/GUI/Immediate/FontIcon.cpp FredEmmott/GUI/Immediate/FontIcon.hpp
  FredEmmott/GUI/Immediate/Label.cpp FredEmmott/GUI/Immediate/Label.hpp
  FredEmmott/GUI/Immediate/PopupWindow.cpp FredEmmott/GUI/Immediate/PopupWindow.hpp
  FredEmmott/GUI/Immediate/Root.cpp FredEmmott/GUI/Immediate/Root.hpp
  FredEmmott/GUI/Immediate/StackPanel.hpp
  FredEmmott/GUI/Immediate/TextBlock.cpp FredEmmott/GUI/Immediate/TextBlock.hpp
  FredEmmott/GUI/Immediate/ToggleSwitch.cpp FredEmmott/GUI/Immediate/ToggleSwitch.hpp
  FredEmmott/GUI/Interpolation/CubicBezier.hpp
  FredEmmott/GUI/Interpolation/Linear.cpp FredEmmott/GUI/Interpolation/Linear.hpp
  FredEmmott/GUI/LinearGradientBrush.cpp FredEmmott/GUI/LinearGradientBrush.hpp
  FredEmmott/GUI/SolidColorBrush.hpp
  FredEmmott/GUI/StaticTheme.cpp FredEmmott/GUI/StaticTheme.hpp
  FredEmmott/GUI/StaticTheme/Resource.hpp
  FredEmmott/GUI/StaticTheme/Theme.hpp
  FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp
  FredEmmott/GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp
  FredEmmott/GUI/Style.cpp FredEmmott/GUI/Style.hpp
  FredEmmott/GUI/StyleProperty.hpp>
  FredEmmott/GUI/StyleTransition.hpp
  FredEmmott/GUI/SystemFont.cpp FredEmmott/GUI/SystemFont.hpp
  FredEmmott/GUI/SystemTheme.cpp FredEmmott/GUI/SystemTheme.hpp
  FredEmmott/GUI/WidgetFont.cpp FredEmmott/GUI/WidgetFont.hpp
  FredEmmott/GUI/Widgets/Button.cpp FredEmmott/GUI/Widgets/Button.hpp
  FredEmmott/GUI/Widgets/Card.cpp FredEmmott/GUI/Widgets/Card.hpp
  FredEmmott/GUI/Widgets/Label.cpp FredEmmott/GUI/Widgets/Label.hpp
  FredEmmott/GUI/Widgets/PopupWIndow.cpp FredEmmott/GUI/Widgets/PopupWindow.hpp
  FredEmmott/GUI/Widgets/StackPanel.cpp FredEmmott/GUI/Widgets/StackPanel.hpp
  FredEmmott/GUI/Widgets/TextBlock.cpp FredEmmott/GUI/Widgets/TextBlock.hpp
  FredEmmott/GUI/Widgets/ToggleSwitch.cpp FredEmmott/GUI/Widgets/ToggleSwitch.hpp
  FredEmmott/GUI/Widgets/ToggleSwitchKnob.cpp FredEmmott/GUI/Widgets/ToggleSwitchKnob.hpp
  FredEmmott/GUI/Widgets/ToggleSwitchThumb.cpp FredEmmott/GUI/Widgets/ToggleSwitchThumb.hpp
  FredEmmott/GUI/Widgets/Widget.cpp
  FredEmmott/GUI/Widgets/Widget.hpp
  FredEmmott/GUI/Widgets/WidgetStyles.cpp FredEmmott/GUI/Widgets/WidgetStyles.hpp
  FredEmmott/GUI/Widgets/Widget_ComputeStyles.cpp
  FredEmmott/GUI/Widgets/Widget_StyleTransitions.cpp
  FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.cpp FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.hpp
  FredEmmott/GUI/detail/font_detail.hpp
  FredEmmott/GUI/detail/immediate/Widget.hpp
  FredEmmott/GUI/detail/immediate_detail.cpp FredEmmott/GUI/detail/immediate_detail.hpp
  FredEmmott/GUI/detail/style_detail.hpp
  FredEmmott/GUI/detail/widget_detail.hpp
  FredEmmott/GUI/events/Event.hpp
  FredEmmott/GUI/events/MouseButton.hpp
  FredEmmott/GUI/events/MouseButtonPressEvent.hpp
  FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp
  FredEmmott/GUI/events/MouseEvent.hpp
  FredEmmott/GUI/events/MouseMoveEvent.hpp
  FredEmmott/GUI/yoga.cpp FredEmmott/GUI/yoga.hpp
  FredEmmott/memory.hpp
  FredEmmott/memory/memory_detail.hpp
  FredEmmott/type_traits/concepts.hpp
  FredEmmott/utility/bitflag_enums.hpp
  FredEmmott/utility/lazy_init.hpp
  FredEmmott/utility/type_tag.hpp
  FredEmmott/GUI/Immediate/ID.hpp
)
target_link_libraries(
  HelloSkia-Win32-Ganesh-D3D12
  PRIVATE
  winui3-themes
  # vpckg
  skia
  unofficial::skia::modules::skunicode_icu
  unofficial::skia::modules::skparagraph
  yoga::yogacore
  # system
  Dcomp
  Dwmapi
  User32
)
target_compile_definitions(
  HelloSkia-Win32-Ganesh-D3D12
  PRIVATE
  "UNICODE=1"
  "_UNICODE=1"
)
target_include_directories(
  HelloSkia-Win32-Ganesh-D3D12
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
