find_package(unofficial-skia CONFIG REQUIRED)
find_package(yoga CONFIG REQUIRED)

# Add our own target to do some changes
add_library(skia INTERFACE)
target_link_libraries(skia INTERFACE unofficial::skia::skia)
target_compile_definitions(
  skia
  INTERFACE
  # Required for compatibility with <Windows.h>
  "NOMINMAX=1"
)

# The default target requires `#include <core/SkCanvas.h>`; also allow `#include <skia/core/SkCanvas.h>` for readability
get_target_property(SKIA_INCLUDE_DIRECTORIES unofficial::skia::skia INTERFACE_INCLUDE_DIRECTORIES)
set(HAVE_SKIA_CORE_CANVAS_H OFF)
foreach (PATH IN LISTS SKIA_INCLUDE_DIRECTORIES)
  if (EXISTS "$PATH}/skia/core/canvas.h")
    set(HAVE_SKIA_CORE_CANVAS_H ON)
    break()
  endif ()
endforeach ()
if (NOT HAVE_SKIA_CORE_CANVAS_H)
  foreach (PATH IN LISTS SKIA_INCLUDE_DIRECTORIES)
    cmake_path(GET PATH FILENAME TAIL)
    if (TAIL STREQUAL "skia")
      cmake_path(GET PATH PARENT_PATH PARENT)
      target_include_directories(skia INTERFACE "${PARENT}")
    endif ()
  endforeach ()
endif ()

add_executable(
  HelloSkia-Win32-Ganesh-D3D12
  WIN32
  Win32-Ganesh-D3D12.cpp
  Win32-Ganesh-D3D12.hpp

  FredEmmott/GUI/Color.cpp FredEmmott/GUI/Color.hpp
  FredEmmott/GUI/Font.cpp FredEmmott/GUI/Font.hpp
  FredEmmott/GUI/Style.cpp FredEmmott/GUI/Style.hpp
  FredEmmott/GUI/Brush.hpp
  FredEmmott/GUI/SolidColorBrush.hpp
  FredEmmott/GUI/Immediate/Button.hpp
  FredEmmott/GUI/Immediate/Card.hpp
  FredEmmott/GUI/Immediate/Label.hpp
  FredEmmott/GUI/Immediate/Root.cpp FredEmmott/GUI/Immediate/Root.hpp
  FredEmmott/GUI/Immediate/StackLayout.hpp
  FredEmmott/GUI/Immediate/concepts.hpp
  FredEmmott/GUI/detail/immediate/Widget.cpp FredEmmott/GUI/detail/immediate/Widget.hpp
  FredEmmott/GUI/events/Event.hpp
  FredEmmott/GUI/events/MouseButton.hpp
  FredEmmott/GUI/events/MouseButtonPressEvent.hpp
  FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp
  FredEmmott/GUI/events/MouseEvent.hpp
  FredEmmott/GUI/events/MouseMoveEvent.hpp
  FredEmmott/GUI/SystemColor.cpp FredEmmott/GUI/SystemColor.hpp
  FredEmmott/GUI/SystemFont.cpp FredEmmott/GUI/SystemFont.hpp
  FredEmmott/GUI/WidgetFont.cpp FredEmmott/GUI/WidgetFont.hpp
  FredEmmott/GUI/Widgets/Button.cpp FredEmmott/GUI/Widgets/Button.hpp
  FredEmmott/GUI/Widgets/Card.cpp FredEmmott/GUI/Widgets/Card.hpp
  FredEmmott/GUI/Widgets/Label.cpp FredEmmott/GUI/Widgets/Label.hpp
  FredEmmott/GUI/Widgets/StackLayout.cpp FredEmmott/GUI/Widgets/StackLayout.hpp
  FredEmmott/GUI/Widgets/Widget.cpp FredEmmott/GUI/Widgets/Widget.hpp
  FredEmmott/GUI/Widgets/WidgetStyles.cpp FredEmmott/GUI/Widgets/WidgetStyles.hpp
  FredEmmott/GUI/detail/font_detail.hpp
  FredEmmott/GUI/detail/immediate_detail.cpp FredEmmott/GUI/detail/immediate_detail.hpp
  FredEmmott/GUI/StaticTheme.cpp FredEmmott/GUI/StaticTheme.hpp
  FredEmmott/GUI/LinearGradientBrush.cpp FredEmmott/GUI/LinearGradientBrush.hpp
  FredEmmott/utility/bitflag_enums.hpp
  FredEmmott/GUI/ActivatedFlag.hpp
  FredEmmott/utility/lazy_init.hpp
  FredEmmott/utility/type_tag.hpp
  FredEmmott/type_traits/concepts.hpp
  FredEmmott/GUI/yoga.hpp
  FredEmmott/memory.hpp
  FredEmmott/memory/memory_detail.hpp
)
target_link_libraries(
  HelloSkia-Win32-Ganesh-D3D12
  PRIVATE
  skia
  yoga::yogacore
  winui3-themes
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
