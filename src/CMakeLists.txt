if(ENABLE_SKIA)
  include(skia)
endif()

find_package(wil CONFIG REQUIRED)

set(CONFIG_HPP "${CMAKE_CURRENT_BINARY_DIR}/include/FredEmmott/GUI/config.hpp")
block()
  set(FUI_ENABLE_SKIA ${ENABLE_SKIA})
  set(FUI_ENABLE_DIRECT2D ${ENABLE_DIRECT2D})
  configure_file(
    "FredEmmott/GUI/config.hpp.in"
    "${CONFIG_HPP}"
    @ONLY
    NEWLINE_STYLE LF
  )
endblock()
add_library(fredemmott-gui-config INTERFACE)
target_include_directories(fredemmott-gui-config INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include")

add_subdirectory(xaml-to-fui-statictheme)

include(lib.cmake)

option(BUILD_DEMO "Build the demo app" ${PROJECT_IS_TOP_LEVEL})
if (BUILD_DEMO)
  include(demo.cmake)
endif()