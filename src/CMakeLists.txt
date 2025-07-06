if (ENABLE_SKIA)
  include(skia)
endif ()

find_package(wil CONFIG REQUIRED)

set(CONFIG_HPP "${CMAKE_CURRENT_BINARY_DIR}/include/FredEmmott/GUI/config.hpp")
block()
  set(BACKEND_COUNT 0)
  foreach (BACKEND IN ITEMS SKIA DIRECT2D)
    set("FUI_ENABLE_${BACKEND}" "${ENABLE_${BACKEND}}")
    if (ENABLE_${BACKEND})
      MATH(EXPR BACKEND_COUNT "${BACKEND_COUNT} + 1")
      set("ENABLE_${BACKEND}_BOOL" true)
    else ()
      set("ENABLE_${BACKEND}_BOOL" false)
    endif ()
  endforeach ()
  configure_file(
    "FredEmmott/GUI/config.hpp.in"
    "${CONFIG_HPP}"
    @ONLY
    NEWLINE_STYLE LF
  )
endblock()
add_library(fredemmott-gui-config INTERFACE)
target_include_directories(
  fredemmott-gui-config
  INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
install(TARGETS fredemmott-gui-config EXPORT exports)
install(
  FILES
  "${CONFIG_HPP}"
  DESTINATION "include/FredEmmott/GUI"
)

add_subdirectory(xaml-to-fui-statictheme)

include(lib.cmake)

option(BUILD_DEMO "Build the demo app" ${PROJECT_IS_TOP_LEVEL})
if (BUILD_DEMO)
  include(demo.cmake)
endif ()