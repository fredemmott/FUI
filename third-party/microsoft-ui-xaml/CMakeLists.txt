find_program(PWSH pwsh DOC "Modern Powershell" REQUIRED)

set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/FredEmmott/GUI/detail")
file(MAKE_DIRECTORY "${OUTPUT_PATH}")
set(HPP_FILE_PATH "${OUTPUT_PATH}/WinUI3Themes.hpp")

add_custom_command(
  OUTPUT "${HPP_FILE_PATH}"
  COMMAND
  "${PWSH}"
  -ExecutionPolicy Bypass
  "${CMAKE_CURRENT_SOURCE_DIR}/xaml-to-cpp.ps1"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common_themeresources_any.xaml"
  "${HPP_FILE_PATH}"
  MAIN_DEPENDENCY
  Common_themeresources_any.xaml
  DEPENDS
  xaml-to-cpp.ps1
)
add_library(winui3-themes INTERFACE "${HPP_FILE_PATH}")
target_include_directories(winui3-themes INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include")