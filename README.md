# Planet Engine

A minimal, Lua-scriptable 3D/2D game engine with an OpenGL renderer, ECS, physics, and audio. Built for cross-platform distribution (Linux, Windows, macOS).

## Features

- **OpenGL 4.1 Core** renderer with shaders, textures, lighting, and post-processing effects (VHS, bloom, CRT, etc.)
- **Lua 5.4 scripting** for game logic, scene management, and entity behavior
- **Entity Component System** with transform, mesh, camera, light, physics, and scripting components
- **Map system** — text-based `.map` files with support for templates, lighting, and meshes
- **Console** — in-game developer console (backtick to toggle) with help, variable inspection, and command execution
- **Physics** — basic 3D physics with gravity, collision, and raycasting
- **Audio** — OpenAL (or FMOD) with 3D positional sound
- **DEXECL** — declarative scripting language for entity configuration
- **Post-processing** — 14 effects (VHS, vignette, bloom, CRT, fisheye, etc.)

## What Works

- ✅ 3D rendering with meshes, textures, materials, lighting (directional, point, flashlight)
- ✅ Lua scripting with full engine API binding
- ✅ ECS with 8 component types
- ✅ Map loading (text format and legacy Lua format)
- ✅ Console with 30+ commands
- ✅ Audio playback (OpenAL on Linux/macOS, OpenAL on Windows)
- ✅ Physics simulation
- ✅ Post-processing effects
- ✅ Maze game project (WASD movement, random maze generation, enemy AI)
- ✅ Cross-compilation: Linux (native), Windows (MinGW), macOS (ARM64)
- ✅ Font rendering with TTF fallback to built-in bitmap font
- ✅ Sprite and text overlay rendering
- ✅ DEXECL configuration files

## What Does Not Work / Incomplete

- ❌ Editor (ncurses TUI) — archived, not built
- ❌ Map editor (`planet_mapper`) — compiled but no usable TUI
- ❌ Kerdata bundler — experimental, not integrated
- ❌ LSP client — incomplete (code present but untested)
- ❌ FMOD audio backend — optional, requires FMOD SDK
- ❌ macOS cross-compilation from Linux — requires osxcross + Xcode SDK
- ❌ Network/multiplayer — not implemented

## Quick Start

### Dependencies

- CMake ≥ 3.20
- OpenGL 4.1 capable GPU
- GLFW 3.x
- OpenAL (Linux: `libopenal-dev`, macOS: built-in framework)
- Lua 5.4 (bundled in `thirdparty/`)
- pthreads

### Build

```bash
cmake -B build
make -C build -j$(nproc)
```

### Run the maze game

```bash
cp -r projects/maze/* build/bin/
cd build/bin && ./planet
```

Controls: WASD — move, Mouse — look, Tab — toggle mouse capture, Backtick — console, Escape — quit.

### Native build on Windows

Requires **Visual Studio 2022** (or 2019) with "Desktop development with C++" workload, or **MinGW-w64** (UCRT64/MSYS2).

Using vcpkg to install dependencies:

```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && .\bootstrap-vcpkg.bat
.\vcpkg install glfw3 openal-soft
cd ..\planet
cmake -B build -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build
```

Or install GLFW and OpenAL manually from their official installers, then:

```powershell
cmake -B build
cmake --build build
```

The maze game and its assets will be in `build\bin\`.

### Cross-compile for Windows (from Linux)

```bash
cmake -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake -DPLANET_BUILD_WINDOWS=ON
make -C build-mingw package_windows
```

### Build for macOS

On an Apple Silicon Mac:

```bash
cmake -B build
make -C build -j$(sysctl -n hw.ncpu) planet
```

## Project Structure

```
├── CMakeLists.txt          # Build system
├── cmake/                  # CMake toolchain files
├── fonts/                  # Bundled font files
├── projects/               # Game projects
│   ├── maze/               # Maze game (primary distributable)
│   └── myproject/          # Template project
├── scripts/                # Build helper scripts
├── src/
│   ├── main.cpp            # Entry point
│   ├── planet/             # Engine core
│   │   ├── audio/          # OpenAL audio
│   │   ├── core/           # Window, input, scene, console, camera
│   │   ├── ecs/            # Entity Component System
│   │   ├── editor/         # [Archived] Editor source
│   │   ├── map/            # Map loader and parser
│   │   ├── mapper/         # [Archived] TUI map editor
│   │   ├── physics/        # Physics engine
│   │   ├── render/         # Renderer, shaders, text, sprites
│   │   └── resource/       # Resource manager
│   ├── lua/                # Lua engine and API bindings
│   └── bundler/            # Kerdata archive tool
└── thirdparty/             # Vendored libraries (Lua, GLAD, stb, glm)
```

## Documentation

See [docs/ENGINE.md](docs/ENGINE.md) for the complete engine API, map format, and console reference.

## License

This project is provided as-is. Third-party libraries (Lua, GLFW, GLAD, stb, glm) are under their respective licenses — see [CREDITS.md](CREDITS.md).
