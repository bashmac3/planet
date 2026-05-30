#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build-mingw/bin"
DIST_DIR="dist/planet-windows"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Copy executable and DLLs
cp "$BUILD_DIR/planet.exe" "$DIST_DIR/"
cp "$BUILD_DIR"/libgcc_s_seh-1.dll "$DIST_DIR/"
cp "$BUILD_DIR"/libstdc++-6.dll "$DIST_DIR/"
cp "$BUILD_DIR"/libwinpthread-1.dll "$DIST_DIR/"

# Copy project files
cp "$BUILD_DIR/project.planet" "$DIST_DIR/"
cp "$BUILD_DIR/manifest.plm" "$DIST_DIR/"

# Copy directories
cp -r "$BUILD_DIR/assets" "$DIST_DIR/"
cp -r "$BUILD_DIR/scripts" "$DIST_DIR/"
cp -r "$BUILD_DIR/maps" "$DIST_DIR/"
cp -r "$BUILD_DIR/dexecl" "$DIST_DIR/"
cp -r "$BUILD_DIR/fonts" "$DIST_DIR/"

# Create archive
cd dist
rm -f planet-windows.tar.gz planet-windows.zip
tar czf planet-windows.tar.gz planet-windows/
cd ..

echo "---"
echo "Distribution ready: dist/planet-windows/ ($(du -sh dist/planet-windows/ | cut -f1))"
echo "Zip: dist/planet-windows.zip"
