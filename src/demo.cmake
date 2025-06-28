add_executable(
  fredemmott-gui-demo
  WIN32
  app.exe.manifest
  demo.cpp
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
