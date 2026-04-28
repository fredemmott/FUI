# Building on Linux

### Additional Linux requirements

GCC 14+ (libstdc++-14 provides `std::ranges::to`).

The dependency lists below are long because most distros don't yet ship SDL3,
so vcpkg builds it from source. SDL3's `x11`, `wayland`, and `ibus` features
each need a handful of `*-devel`/`-dev` headers at SDL build time, and the
autoconf/automake/libtool chain comes in via SDL3 → dbus → libsystemd →
libxcrypt's autotools chain.

**Ubuntu 24.04 (Debian-family):**

```bash
sudo apt install \
  build-essential g++-14 git curl zip unzip tar pkg-config ninja-build cmake \
  autoconf autoconf-archive automake libtool-bin libltdl-dev \
  libicu-dev \
  libx11-dev libxft-dev libxext-dev libxkbcommon-dev libxrandr-dev libxi-dev libxcursor-dev libxtst-dev \
  libwayland-dev wayland-protocols libdecor-0-dev libegl1-mesa-dev \
  libibus-1.0-dev \
  libvulkan-dev vulkan-tools \
  libfontconfig1-dev libfreetype-dev \
  libharfbuzz-dev libdbus-1-dev
  ```

**Fedora 43 (Red Hat-family):**

```bash
sudo dnf install \
  gcc gcc-c++ git curl zip unzip tar pkgconf-pkg-config ninja-build cmake \
  autoconf autoconf-archive automake libtool \
  libicu-devel \
  libX11-devel libXft-devel libXext-devel libxkbcommon-devel libXrandr-devel libXi-devel libXcursor-devel libXtst-devel \
  wayland-devel wayland-protocols-devel libdecor-devel mesa-libEGL-devel \
  ibus-devel \
  vulkan-loader-devel vulkan-tools \
  fontconfig-devel freetype-devel \
  harfbuzz-devel dbus-devel 
```

**Arch Linux:**

```bash
sudo pacman -S --needed \
  base-devel git curl zip unzip tar pkgconf ninja cmake \
  autoconf autoconf-archive automake libtool \
  icu \
  libx11 libxft libxext libxkbcommon libxrandr libxi libxcursor libxtst \
  wayland wayland-protocols libdecor mesa \
  ibus \
  vulkan-headers vulkan-icd-loader vulkan-tools \
  fontconfig freetype2 \
  harfbuzz dbus 
```

#### Segoe UI fonts (optional)
Linux will default to internal fonts,
FUI's stock theme expects "Segoe UI Variable Text" / "Segoe Fluent Icons".
Without those installed, the runtime falls back to "Helvetica" → Skia's
internal default, which renders as generic sans-serif and lacks the icon
glyphs (FontIcon characters render as empty boxes; Debug builds also trip
a `FUI_ASSERT` because the missing-glyph width doesn't match the font
size). Install Microsoft's Segoe UI family on Linux via a community
repackage:

```bash
git clone https://github.com/mrbvrz/segoe-ui-linux
cd segoe-ui-linux
chmod +x install.sh
./install.sh
```

Then `fc-cache -fv` if `install.sh` doesn't already do it. Verify with
`fc-match "Segoe UI Variable Text"`.