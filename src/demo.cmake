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
if (WIN32)
  target_sources(
    fredemmott-gui-demo
    PRIVATE
    demo_win32.cpp
    demo_win32.hpp
  )
  # Used for VRAMTexture() demo even if using Skia
  #
  # If FUI isn't using D3D11+D2D, this tests cross-API texture and fence behavior
  foreach (NAME IN ITEMS DXGI D3d11)
    find_library("${NAME}_PATH" "${NAME}" REQUIRED)
    target_link_libraries(fredemmott-gui PRIVATE "${${NAME}_PATH}")
  endforeach ()
  target_include_directories(
    fredemmott-gui-demo
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endif ()

set(NAME fui-demo)
if (ENABLE_SKIA)
  string(APPEND NAME "-skia")
endif ()
if (ENABLE_DIRECT2D)
  string(APPEND NAME "-direct2d")
endif ()
set_target_properties(
  fredemmott-gui-demo
  PROPERTIES OUTPUT_NAME "${NAME}"
)

if (MSVC)
  target_link_options(
    fredemmott-gui-demo
    PRIVATE
    # Incremental linking makes rebuilding quicker, but is a big size increase.
    # Don't do this in release builds.
    #
    # - we generally don't want to rebuild release builds with small changes
    # - smaller binaries are good
    # - as well as the direct increase, incremental linking breaks the SizeBench tool
    "$<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>"
    # COMDAT folding; drops off another big chunk
    "$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>"
    # Remove unused data and variables
    # In particular, undefine all the RuntimeClass_ and InterfaceName_ constants from WinRT ABI
    "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>"
  )
endif ()
