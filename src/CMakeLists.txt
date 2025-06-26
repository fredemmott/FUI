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
add_subdirectory(FredEmmott)

add_executable(
  fredemmott-gui-demo
  WIN32
  app.exe.manifest
  main.cpp
)
target_link_libraries(
  fredemmott-gui-demo
  PRIVATE
  fredemmott-gui
)
target_include_directories(
  fredemmott-gui-demo
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(NAME fui-demo)
if (ENABLE_SKIA)
  string(APPEND NAME "-skia")
endif()
if (ENABLE_DIRECT2D)
  string(APPEND NAME "-direct2d")
endif()
set_target_properties(
  fredemmott-gui-demo
  PROPERTIES OUTPUT_NAME "${NAME}"
)