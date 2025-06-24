if(ENABLE_SKIA)
  include(skia)
endif()

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
  HelloSkia-Win32-Ganesh-D3D12
  WIN32
  app.exe.manifest
  main.cpp
)
target_link_libraries(
  HelloSkia-Win32-Ganesh-D3D12
  PRIVATE
  fredemmott-gui
)
target_include_directories(
  HelloSkia-Win32-Ganesh-D3D12
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
