if (NOT WIN32)
  return()
endif ()

find_program(MAGICK_EXE NAMES magick)
if (MAGICK_EXE)
  set(ENABLE_ICON_DEFAULT ON)
else ()
  set(ENABLE_ICON_DEFAULT OFF)
endif ()
option(ENABLE_ICON "Use custom icon" "${ENABLE_ICON_DEFAULT}")
if (NOT MAGICK_EXE)
  message(WARNING "Not creating application icon, did not find ImageMagick ('magick')")
  if (ENABLE_ICON)
    message(FATAL_ERROR "Icon force-enabled, but can't be created")
  endif ()
  return()
endif ()
if (NOT ENABLE_ICON)
  return()
endif ()

# Ideal sizes:
#
# https://learn.microsoft.com/en-us/windows/apps/design/style/iconography/app-icon-construction
set(ICON_SIZES 16 20 24 30 32 36 48 60 64 72 80 96 128 256)
set(OUT_DIR "${CMAKE_BINARY_DIR}/assets")

# Ensure the output directory exists at configuration time
file(MAKE_DIRECTORY "${OUT_DIR}")

set(PNG_FILES "")

foreach (size IN LISTS ICON_SIZES)
  set(outFile "${OUT_DIR}/AppList.targetsize-${size}.png")
  list(APPEND PNG_FILES "${outFile}")

  # Calculate padding and font size
  if (size GREATER_EQUAL 32)
    math(EXPR padding 2)
  else ()
    set(padding 0)
  endif ()

  math(EXPR fontSizeDim "${size} - ${padding}")
  set(fontSize "${fontSizeDim}x${fontSizeDim}")

  # Calculate max coordinate for drawing
  math(EXPR max "${size} - 1")

  # Command to generate individual PNG
  add_custom_command(
    OUTPUT "${outFile}"
    COMMAND ${MAGICK_EXE} -size "${size}x${size}" xc:none
    +antialias
    -fill "rgba(0,255,0,0.1)" -draw "rectangle 0,0,${max},${max}"
    -stroke black -strokewidth 1 -draw "rectangle 0,0,${max},${max}"
    -stroke white -strokewidth 1 -draw "stroke-dashoffset 0.5 stroke-dasharray 1 1 rectangle 0,0,${max},${max}"
    -gravity center
    -antialias
    "("
    -size "${fontSize}" -background none -font Verdana -fill white "label:${size}"
    "(" +clone -background black -shadow "100x1+0+0" ")" +swap -composite
    ")"
    -composite
    -colors 16
    "${outFile}"
    COMMENT "Generating icon asset: ${size}px"
    VERBATIM
  )
endforeach ()

# Final command to bundle PNGs into the .ico file
set(FINAL_ICO "${OUT_DIR}/demoicon.ico")

add_custom_command(
  OUTPUT "${FINAL_ICO}"
  COMMAND ${MAGICK_EXE} ${PNG_FILES} -background transparent -compress zip "${FINAL_ICO}"
  DEPENDS ${PNG_FILES}
  COMMENT "Bundling icons into ${FINAL_ICO}"
  VERBATIM
)

# Create a target so these commands actually run during the build
add_custom_target(generate_app_icons ALL DEPENDS "${FINAL_ICO}")

set(ICO_RC "${OUT_DIR}/demoicon.rc")
file(
  WRITE "${ICO_RC}"
  "#define IDL_ICON1 101\n"
  "IDL_ICON1 ICON DISCARDABLE \"demoicon.ico\"\n"
)
add_dependencies(fredemmott-gui-demo generate_app_icons)
target_sources(fredemmott-gui-demo PRIVATE "${ICO_RC}")