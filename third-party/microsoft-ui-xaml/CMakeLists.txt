include(skia)

find_program(PWSH pwsh DOC "Modern Powershell" REQUIRED)

set(OUTPUT_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated")

add_library(winui3-themes INTERFACE)
function(add_winui3_theme_file COMPONENT SOURCE EXTENSION)
  set(options)
  set(oneValueArgs DESTINATION)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (arg_DESTINATION)
    set(OUTPUT_FILE_PATH "${arg_DESTINATION}")
  else ()
    message(FATAL_ERROR "DESTINATION must be specified")
  endif ()
  cmake_path(GET OUTPUT_FILE_PATH PARENT_PATH OUTPUT_DIRECTORY)
  file(MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")

  set(CPP_TARGET "winui3-themes-${COMPONENT}-Cpp")
  set(HPP_TARGET "winui3-themes-${COMPONENT}-Hpp")
  set(TARGET "winui3-themes-${COMPONENT}-${EXTENSION}")

  add_custom_command(
    OUTPUT "${OUTPUT_FILE_PATH}"
    COMMAND
    "${PWSH}"
    -ExecutionPolicy Bypass
    "${CMAKE_CURRENT_SOURCE_DIR}/xaml-to-cpp.ps1"
    "${COMPONENT}"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common_themeresources_any.xaml"
    "${OUTPUT_FILE_PATH}"
    -Mode ${EXTENSION}
    MAIN_DEPENDENCY
    Common_themeresources_any.xaml
    DEPENDS
    xaml-to-cpp.ps1
  )
  message(STATUS "Added rule to generate ${OUTPUT_FILE_PATH}")
  if ("${EXTENSION}" STREQUAL "Hpp")
    add_library("${HPP_TARGET}" INTERFACE "${OUTPUT_FILE_PATH}")
    target_include_directories("${HPP_TARGET}" INTERFACE "${OUTPUT_BASE_PATH}")
  else ()
    add_library("${CPP_TARGET}" STATIC "${OUTPUT_FILE_PATH}")
    target_link_libraries(
      "${CPP_TARGET}"
      PRIVATE
      "${HPP_TARGET}"
      skia
    )
    target_include_directories(
      "${CPP_TARGET}"
      PRIVATE
      "${CMAKE_SOURCE_DIR}/src"
    )
  endif ()
  target_link_libraries(winui3-themes INTERFACE "${TARGET}")
endfunction()

function(add_winui3_component_theme NAME SOURCE)
  cmake_path(ABSOLUTE_PATH SOURCE)

  set(PUBLIC_HPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/${NAME}.hpp")
  set(PUBLIC_CPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/${NAME}.cpp")

  add_winui3_theme_file(${NAME} "${SOURCE}" Hpp DESTINATION "${PUBLIC_HPP}")
  add_winui3_theme_file(${NAME} "${SOURCE}" Cpp DESTINATION "${PUBLIC_CPP}")
endfunction()
add_winui3_component_theme(Common "Common_themeresources_any.xaml")
