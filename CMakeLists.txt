cmake_minimum_required(VERSION 3.25)

option(ENABLE_IMPLICIT_BACKENDS "Automatically enable available backends" ON)
option(ENABLE_SKIA "Build with support for Skia" ${ENABLE_IMPLICIT_BACKENDS})
if(ENABLE_SKIA)
  list(APPEND VCPKG_MANIFEST_FEATURES "skia")
endif()
option(ENABLE_DIRECT2D "Build with support for Direct2D+DirectWrite+D3D11" ${ENABLE_IMPLICIT_BACKENDS})
if (ENABLE_DIRECT2D)
  list(APPEND VCPKG_MANIFEST_FEATURES "direct2d")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
  # Standard C++ exception behavior
  "/EHsc"
  # Include content and marker in error messages
  "/diagnostics:caret"
  # Source and execution charset
  "/utf-8"
)

if ("${VCPKG_TARGET_TRIPLET}" MATCHES "-static$")
  # https://github.com/microsoft/WindowsAppSDK/blob/main/docs/Coding-Guidelines/HybridCRT.md
  set(
    CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )
  add_link_options(
    "/DEFAULTLIB:ucrt$<$<CONFIG:Debug>:d>.lib" # include the dynamic UCRT
    "/NODEFAULTLIB:libucrt$<$<CONFIG:Debug>:d>.lib" # remove the static UCRT
  )
elseif (ENABLE_SKIA)
  message(FATAL_ERROR "Advanced text features require SkParagraph, which is only available with vcpkg static triplets}")
endif ()

set(
  CMAKE_TOOLCHAIN_FILE
  "${CMAKE_SOURCE_DIR}/third-party/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file"
)

project(FUI VERSION 0.0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)
