include(skia)

find_program(PWSH pwsh DOC "Modern Powershell" REQUIRED)

set(OUTPUT_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated")

add_library(winui3-themes INTERFACE)
function(add_winui3_theme_file COMPONENT EXTENSION)
  set(options)
  set(oneValueArgs DESTINATION PARENT)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if ("PARENT" IN_LIST ARGN)
    set(PARENT_THEME "${arg_PARENT}")
  else ()
    set(PARENT_THEME "Common")
  endif ()

  set(SOURCES)
  if (NOT arg_SOURCES)
    set(arg_SOURCES "${COMPONENT}_themeresources.xaml")
  endif ()
  foreach (SOURCE IN LISTS arg_SOURCES)
    cmake_path(ABSOLUTE_PATH SOURCE)
    list(APPEND SOURCES "${SOURCE}")
  endforeach ()

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
    -Component "${COMPONENT}"
    -InputFiles "${SOURCES}"
    -OutputFile "${OUTPUT_FILE_PATH}"
    -Parent "${PARENT_THEME}"
    -Mode ${EXTENSION}
    DEPENDS
    xaml-to-cpp.ps1
    "${SOURCES}"
    VERBATIM
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

function(add_winui3_component_theme COMPONENT)
  set(PUBLIC_HPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/${COMPONENT}.hpp")
  set(PUBLIC_CPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/${COMPONENT}.cpp")

  add_winui3_theme_file(${COMPONENT} Hpp DESTINATION "${PUBLIC_HPP}" ${ARGN})
  add_winui3_theme_file(${COMPONENT} Cpp DESTINATION "${PUBLIC_CPP}" ${ARGN})
endfunction()
add_winui3_component_theme(Generic SOURCES "generic.xaml" PARENT "")
add_winui3_component_theme(Common SOURCES "Common_themeresources_any.xaml" "Common_themeresources.xaml" PARENT Generic)
add_winui3_component_theme(Deprecated)

add_winui3_component_theme(Button PARENT Deprecated)

add_winui3_component_theme(ToggleSwitch)
