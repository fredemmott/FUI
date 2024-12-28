include(skia)

find_program(PWSH pwsh DOC "Modern Powershell" REQUIRED)

set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/FredEmmott/GUI/detail/WinUI3Themes")
file(MAKE_DIRECTORY "${OUTPUT_PATH}")

add_library(winui3-themes INTERFACE)
function(add_winui3_theme_hpp MODE EXTENSION)
  set(MODE_SWITCH "-${MODE}${EXTENSION}")
  string(TOLOWER "${MODE}.${EXTENSION}" OUTPUT_FILE_NAME)

  set(OUTPUT_FILE_PATH "${OUTPUT_PATH}/${OUTPUT_FILE_NAME}")

  add_custom_command(
    OUTPUT "${OUTPUT_FILE_PATH}"
    COMMAND
    "${PWSH}"
    -ExecutionPolicy Bypass
    "${CMAKE_CURRENT_SOURCE_DIR}/xaml-to-cpp.ps1"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common_themeresources_any.xaml"
    "${OUTPUT_FILE_PATH}"
    "${MODE_SWITCH}"
    MAIN_DEPENDENCY
    Common_themeresources_any.xaml
    DEPENDS
    xaml-to-cpp.ps1
  )
  message(STATUS "Added rule to generate ${OUTPUT_FILE_PATH}")
  if ("${EXTENSION}" STREQUAL "Hpp")
    add_library(winui3-themes-${MODE}Hpp INTERFACE "${OUTPUT_FILE_PATH}")
    target_include_directories(winui3-themes-${MODE}Hpp INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include")
  else ()
    add_library(winui3-themes-${MODE}Cpp STATIC "${OUTPUT_FILE_PATH}")
    target_link_libraries(
      winui3-themes-${MODE}Cpp
      PRIVATE
      winui3-themes-${MODE}Hpp
      skia
    )
    target_include_directories(
      winui3-themes-${MODE}Cpp
      PRIVATE
      "${CMAKE_SOURCE_DIR}/src"
    )
  endif ()
  target_link_libraries(winui3-themes INTERFACE winui3-themes-${MODE}${EXTENSION})
endfunction()
add_winui3_theme_hpp(Macros Hpp)
add_winui3_theme_hpp(Enums Hpp)
add_winui3_theme_hpp(Types Hpp)
add_winui3_theme_hpp(Themes Hpp)
add_winui3_theme_hpp(Themes Cpp)
