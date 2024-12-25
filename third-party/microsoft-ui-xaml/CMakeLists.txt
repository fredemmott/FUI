find_program(PWSH pwsh DOC "Modern Powershell" REQUIRED)

set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/FredEmmott/GUI/detail/WinUI3Themes")
file(MAKE_DIRECTORY "${OUTPUT_PATH}")

add_library(winui3-themes INTERFACE)
target_include_directories(winui3-themes INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include")
function(add_winui3_theme_hpp MODE)
  set(HPP_FILE_PATH "${OUTPUT_PATH}/${MODE}.hpp")

  add_custom_command(
    OUTPUT "${HPP_FILE_PATH}"
    COMMAND
    "${PWSH}"
    -ExecutionPolicy Bypass
    "${CMAKE_CURRENT_SOURCE_DIR}/xaml-to-cpp.ps1"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common_themeresources_any.xaml"
    "${HPP_FILE_PATH}"
    "-${MODE}"
    MAIN_DEPENDENCY
    Common_themeresources_any.xaml
    DEPENDS
    xaml-to-cpp.ps1
  )
  message(STATUS "Added rule to generate ${HPP_FILE_PATH}")
  add_library(winui3-themes-${MODE} INTERFACE "${HPP_FILE_PATH}")
  target_link_libraries(winui3-themes INTERFACE winui3-themes-${MODE})
endfunction()
add_winui3_theme_hpp(Macros)
add_winui3_theme_hpp(Enums)
add_winui3_theme_hpp(Types)
add_winui3_theme_hpp(Themes)
