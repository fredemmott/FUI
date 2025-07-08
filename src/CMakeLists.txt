if (ENABLE_SKIA)
  include(skia)
endif ()

find_package(wil CONFIG REQUIRED)

set(CONFIG_HPP "${CMAKE_CURRENT_BINARY_DIR}/include/FredEmmott/GUI/config.hpp")
block()
  configure_file(
    "FredEmmott/GUI/config.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.hpp.configured"
    @ONLY
    NEWLINE_STYLE LF
  )
  file(
    GENERATE
    OUTPUT "${CONFIG_HPP}"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.hpp.configured"
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