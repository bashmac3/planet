#!/bin/bash
# Setup script for Planet Engine dependencies
# Downloads third-party source files (Lua, stb)
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
THIRDPARTY="$SCRIPT_DIR/thirdparty"

echo "=== Planet Engine Setup ==="
echo ""

# Download Lua 5.4.7
LUA_VERSION="5.4.7"
echo "[1/2] Downloading Lua $LUA_VERSION..."

if [ ! -f "$THIRDPARTY/lua/src/lua.h" ]; then
    curl -L -o /tmp/lua-$LUA_VERSION.tar.gz "https://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz"
    tar -xzf /tmp/lua-$LUA_VERSION.tar.gz -C /tmp/
    cp /tmp/lua-$LUA_VERSION/src/* "$THIRDPARTY/lua/src/"
    rm -rf /tmp/lua-$LUA_VERSION /tmp/lua-$LUA_VERSION.tar.gz
    echo "   Lua $LUA_VERSION downloaded."
else
    echo "   Lua already present."
fi

# Download stb_image.h and stb_truetype.h
echo "[2/2] Downloading stb headers..."
if [ ! -f "$THIRDPARTY/stb/stb_image.h" ]; then
    curl -L -o "$THIRDPARTY/stb/stb_image.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
    echo "   stb_image.h downloaded."
else
    echo "   stb_image.h already present."
fi
if [ ! -f "$THIRDPARTY/stb/stb_truetype.h" ]; then
    curl -L -o "$THIRDPARTY/stb/stb_truetype.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h"
    echo "   stb_truetype.h downloaded."
else
    echo "   stb_truetype.h already present."
fi

echo ""
echo "Setup complete!"
echo ""
echo "To build:"
echo "  cmake -B build"
echo "  make -C build -j\$(nproc)"
echo ""
echo "System dependencies required:"
echo "  - CMake 3.20+"
echo "  - OpenGL 4.1 capable GPU"
echo "  - GLFW3 (libglfw3-dev on Linux)"
echo "  - OpenAL (libopenal-dev on Linux, built-in on macOS)"
echo "  - A C++17 compiler (GCC 9+ or Clang 10+)"
echo ""
echo "Optional dependencies:"
echo "  - FMOD Core API (https://www.fmod.com/download)"
echo "    Build with: cmake -B build -DPLANET_USE_FMOD=ON"
echo ""
echo "For Windows cross-compilation:"
echo "  Requires MinGW-w64 toolchain."
echo "  cmake -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake -DPLANET_BUILD_WINDOWS=ON"
echo "  make -C build-mingw package_windows"
