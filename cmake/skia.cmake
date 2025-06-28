include_guard(GLOBAL)

find_package(unofficial-skia CONFIG REQUIRED)

# Add our own customized target
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
      target_include_directories(skia INTERFACE "$<BUILD_INTERFACE:${PARENT}>")
    endif ()
  endforeach ()
endif ()
install(TARGETS skia EXPORT exports)