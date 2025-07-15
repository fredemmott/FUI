# Use `fmt` instead of `<format>` for named arguments in codegen
find_package(fmt CONFIG REQUIRED)
find_package(tinyxml CONFIG REQUIRED)
find_package(yoga CONFIG REQUIRED)

add_executable(
  xaml-to-fui-statictheme
  EXCLUDE_FROM_ALL
  ../app.exe.manifest # Force UTF-8 ACP
  main.cpp
  AliasMap.cpp AliasMap.hpp
  GetAlias.cpp GetAlias.hpp
  GetBoolean.cpp
  GetBoolean.hpp
  GetAcrylicBrush.cpp GetAcrylicBrush.hpp
  GetColor.cpp GetColor.hpp
  GetCornerRadius.cpp GetCornerRadius.hpp
  GetCpp.cpp GetCpp.hpp
  GetHexColorValue.cpp GetHexColorValue.hpp
  GetHpp.cpp GetHpp.hpp
  GetLinearGradientBrush.cpp GetLinearGradientBrush.hpp
  GetNumber.cpp GetNumber.hpp
  GetResources.cpp GetResources.hpp
  GetSolidColorBrush.cpp GetSolidColorBrush.hpp
  GetString.cpp GetString.hpp
  GetThickness.cpp GetThickness.hpp
  ResolveColorReference.cpp ResolveColorReference.hpp
  Resource.hpp
  SortResources.cpp SortResources.hpp
)
install(TARGETS xaml-to-fui-statictheme)

target_link_libraries(
  xaml-to-fui-statictheme
  PRIVATE
  fmt::fmt-header-only
  unofficial-tinyxml::unofficial-tinyxml
)

target_compile_definitions(
  xaml-to-fui-statictheme
  PRIVATE
  "UNICODE=1"
  "_UNICODE=1"
)

set(OUTPUT_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated")

add_library(winui3-themes INTERFACE)
function(add_winui3_theme_file COMPONENT)
  set(options)
  set(oneValueArgs CPP_OUTPUT HPP_OUTPUT DETAIL_HPP_OUTPUT PARENT)
  set(multiValueArgs SOURCES PUBLIC_INCLUDE_DIRECTORIES)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if ("PARENT" IN_LIST ARGN)
    set(PARENT_THEME "${arg_PARENT}")
    if (PARENT_THEME STREQUAL "")
      set(PARENT_THEME "NONE")
    endif ()
  else ()
    set(PARENT_THEME "Common")
  endif ()

  set(SOURCES)
  if (NOT arg_SOURCES)
    set(arg_SOURCES "${COMPONENT}_themeresources.xaml")
  endif ()
  foreach (SOURCE IN LISTS arg_SOURCES)
    cmake_path(ABSOLUTE_PATH SOURCE BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/third-party/microsoft-ui-xaml")
    list(APPEND SOURCES "${SOURCE}")
  endforeach ()

  cmake_path(GET arg_CPP_OUTPUT PARENT_PATH CPP_OUTPUT_DIRECTORY)
  file(MAKE_DIRECTORY "${CPP_OUTPUT_DIRECTORY}")
  cmake_path(GET arg_HPP_OUTPUT PARENT_PATH HPP_OUTPUT_DIRECTORY)
  file(MAKE_DIRECTORY "${HPP_OUTPUT_DIRECTORY}")
  cmake_path(GET arg_DETAIL_HPP_OUTPUT PARENT_PATH DETAIL_HPP_OUTPUT_DIRECTORY)
  file(MAKE_DIRECTORY "${DETAIL_HPP_OUTPUT_DIRECTORY}")

  set(TARGET "fredemmott-gui-StaticTheme-${COMPONENT}")

  add_custom_command(
    OUTPUT
    "${arg_CPP_OUTPUT}"
    "${arg_HPP_OUTPUT}"
    "${arg_DETAIL_HPP_OUTPUT}"
    COMMAND
    "$<TARGET_FILE:xaml-to-fui-statictheme>"
    --cpp-output "${arg_CPP_OUTPUT}"
    --hpp-output "${arg_HPP_OUTPUT}"
    --detail-hpp-output "${arg_DETAIL_HPP_OUTPUT}"
    --parent "${PARENT_THEME}"
    "${COMPONENT}"
    "${SOURCES}"
    DEPENDS
    xaml-to-fui-statictheme
    "${SOURCES}"
    "${CMAKE_CURRENT_LIST_FILE}"
    VERBATIM
    COMMAND_EXPAND_LISTS
  )
  add_library(
    "${TARGET}"
    STATIC
    "${arg_CPP_OUTPUT}"
  )
  target_sources(
    "${TARGET}"
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
    "${OUTPUT_BASE_PATH}"
    FILES
    "${arg_HPP_OUTPUT}"
    "${arg_DETAIL_HPP_OUTPUT}"
  )
  target_link_libraries(
    "${TARGET}"
    PRIVATE
    fredemmott-gui-config
    WIL::WIL
    yoga::yogacore
  )
  if (ENABLE_SKIA)
    target_link_libraries("${TARGET}" PRIVATE skia)
  endif ()
  target_include_directories(
    "${TARGET}"
    PUBLIC
    "$<BUILD_INTERFACE:${HPP_OUTPUT_DIRECTORY}>"
    "$<BUILD_INTERFACE:${arg_PUBLIC_INCLUDE_DIRECTORIES}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
  )
  target_link_libraries(winui3-themes INTERFACE "${TARGET}")

  if (NOT PARENT_THEME STREQUAL "NONE")
    target_link_libraries("${TARGET}" PUBLIC "fredemmott-gui-StaticTheme-${PARENT_THEME}")
  endif ()

  install(TARGETS "${TARGET}" EXPORT exports FILE_SET HEADERS)
endfunction()

function(add_winui3_component_theme COMPONENT)
  set(PUBLIC_HPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/${COMPONENT}.hpp")
  set(PUBLIC_CPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/${COMPONENT}.cpp")
  set(DETAIL_HPP "${OUTPUT_BASE_PATH}/FredEmmott/GUI/StaticTheme/detail/${COMPONENT}.hpp")

  add_winui3_theme_file(
    ${COMPONENT}
    CPP_OUTPUT "${PUBLIC_CPP}"
    HPP_OUTPUT "${PUBLIC_HPP}"
    DETAIL_HPP_OUTPUT "${DETAIL_HPP}"
    PUBLIC_INCLUDE_DIRECTORIES
    "${OUTPUT_BASE_PATH}"
    ${ARGN}
  )
endfunction()

add_winui3_component_theme(Generic SOURCES "generic.xaml" PARENT "")
add_winui3_component_theme(
  Common
  SOURCES
  "Common_themeresources_any.xaml"
  "Common_themeresources.xaml"
  "CornerRadius_themeresources.xaml"
  "AcrylicBrush_themeresources.xaml"
  PARENT Generic
)

add_winui3_component_theme(Deprecated PARENT Common)
add_winui3_component_theme(Button PARENT Deprecated)
add_winui3_component_theme(ComboBox PARENT Button)

add_winui3_component_theme(CheckBox PARENT Common)
add_winui3_component_theme(ContentDialog PARENT Common)
add_winui3_component_theme(HyperlinkButton PARENT Common)
add_winui3_component_theme(RadioButton PARENT Common)
add_winui3_component_theme(RadioButtons PARENT Common)
add_winui3_component_theme(ScrollBar PARENT Common)
add_winui3_component_theme(ScrollView PARENT Common)
add_winui3_component_theme(ToggleSwitch PARENT Common)

install(TARGETS winui3-themes EXPORT exports)
