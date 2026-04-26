#!/usr/bin/env bash
# Build the FUI Linux demo.
#
# Usage:
#   ./build.sh                          # default preset: "Linux - Debug"
#   ./build.sh "Linux - Skia - Debug"
#   ./build.sh "Linux - Skia - Release"
#
# Build directory is derived from the preset name so different presets do
# not stomp each other:
#   Linux - Skia - Debug     -> FUI/build-linux-skia
#   Linux - Skia - Release   -> FUI/build-linux-skia-release
set -euo pipefail

PRESET="${1:-Linux - Skia - Debug}"

# Strip the leading "Linux -" prefix, lowercase, collapse separators, then
# drop the implicit "debug" suffix so the default Debug preset maps to a
# clean "build-linux" rather than "build-linux-debug".
SUFFIX="$(echo "$PRESET" \
  | sed 's/^Linux *-* *//' \
  | tr '[:upper:] ' '[:lower:]-' \
  | sed 's/--*/-/g; s/^-//; s/-$//')"
SUFFIX="${SUFFIX%debug}"
SUFFIX="${SUFFIX%-}"

if [[ -z "$SUFFIX" ]]; then
  BUILD_SUBDIR="build-linux"
else
  BUILD_SUBDIR="build-linux-$SUFFIX"
fi

SCRIPT_DIR="."
FUI_DIR="$SCRIPT_DIR"
BUILD_DIR="$FUI_DIR/$BUILD_SUBDIR"
VCPKG_DIR="$FUI_DIR/third-party/vcpkg"

if [[ ! -x "$VCPKG_DIR/vcpkg" ]]; then
  echo "==> Bootstrapping vcpkg"
  echo "(cd "$VCPKG_DIR" && ./bootstrap-vcpkg.sh)"
  (cd "$VCPKG_DIR" && ./bootstrap-vcpkg.sh)
fi

echo "==> Configuring ($PRESET) -> $BUILD_DIR"
cmake -S "$FUI_DIR" --preset "$PRESET" -B "$BUILD_DIR"

echo "==> Building"
cmake --build "$BUILD_DIR" --parallel

echo "==> Done. Demo binary: $BUILD_DIR/src/fui-demo*"
