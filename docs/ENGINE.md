# Planet Engine Documentation

## Architecture Overview

The engine is organized around a singleton-based core loop:

```
main.cpp → Engine::Run()
  ├── Window (GLFW)
  ├── Input
  ├── Renderer → SpriteRenderer, ModelRenderer, TextRenderer
  ├── PostProcessor (FBO + post-effects)
  ├── Audio (OpenAL or FMOD)
  ├── Physics
  ├── Scene (entity tree)
  ├── LuaRuntime (game scripts)
  ├── Console (in-game dev console)
  ├── SystemManager (ECS systems)
  └── ResourceManager
```

The engine runs a tight loop: `PollEvents → Lua onUpdate → Physics → Scene → Render → SwapBuffers`.

## Lua API

The engine exposes a global Lua table with the following modules:

### Core

| Function | Description |
|----------|-------------|
| `Engine.quit()` | Exit the application |
| `Engine.setTargetFPS(n)` | Limit framerate |
| `Engine.getDeltaTime()` | Frame delta in seconds |
| `Engine.getElapsedTime()` | Time since start |
| `Engine.getAverageFPS()` | Measured FPS |
| `Engine.setVSync(bool)` | Toggle vsync |
| `Engine.setFullscreen(bool)` | Toggle fullscreen |
| `Engine.setClearColor(r,g,b,a)` | Background color |

### Input

| Function | Description |
|----------|-------------|
| `Input.getKey(key)` | Is key held (string name) |
| `Input.getKeyDown(key)` | Key pressed this frame |
| `Input.getKeyUp(key)` | Key released this frame |
| `Input.getMouseX()` / `getMouseY()` | Mouse position |
| `Input.getMouseDX()` / `getMouseDY()` | Mouse delta |
| `Input.getMouseButton(n)` | Mouse button state |
| `Input.setMouseLock(bool)` | Lock cursor to window |
| `Input.isMouseLocked()` | Cursor locked? |
| `Input.getScrollX()` / `getScrollY()` | Scroll wheel delta |

Key names: `"w"`, `"a"`, `"s"`, `"d"`, `"space"`, `"escape"`, `"tab"`, `"grave_accent"`, `"enter"`, `"shift"` etc.

### ECS

| Function | Description |
|----------|-------------|
| `ECS.createEntity(name)` | Create a new entity, returns ID |
| `ECS.removeEntity(id)` | Remove entity |
| `ECS.getEntity(id)` | Get entity by ID |
| `ECS.findEntity(name)` | Find entity by name |
| `ECS.getEntityCount()` | Total entities |
| `ECS.getEntityName(id)` | Get entity name |
| `ECS.entityAddComponent(id, type, props)` | Add component |
| `ECS.entityGetComponent(id, type)` | Get component table |
| `ECS.entityGetPosition(id)` | Returns `{x,y,z}` |
| `ECS.entitySetPosition(id, x,y,z)` | Set position |
| `ECS.entityGetRotation(id)` | Returns `{x,y,z,w}` quaternion |
| `ECS.entitySetRotation(id, x,y,z,w)` | Set rotation quaternion |
| `ECS.entityGetScale(id)` | Returns `{x,y,z}` |
| `ECS.entitySetScale(id, x,y,z)` | Set scale |
| `ECS.entitySetParent(childId, parentId)` | Parent entity |
| `ECS.entitySetTexture(id, path)` | Set mesh texture |
| `ECS.entitySetColor(id, r,g,b,a)` | Set mesh color |

Component types: `"transform"`, `"mesh"`, `"camera"`, `"light"`, `"physics"`, `"script"`, `"portal"`, `"listener"`, `"point_light"`

### Camera

| Function | Description |
|----------|-------------|
| `Camera.create()` | Create a new camera, returns handle |
| `Camera.setActive(handle)` | Set as active camera |
| `Camera.setPosition(h, x,y,z)` | Set camera position |
| `Camera.setRotation(h, x,y,z)` | Set euler rotation |
| `Camera.setFov(h, fov)` | Field of view (degrees) |
| `Camera.setNearPlane(h, n)` | Near clip plane |
| `Camera.setFarPlane(h, f)` | Far clip plane |
| `Camera.setType(h, type)` | `"perspective"` or `"orthographic"` |

### Renderer

| Function | Description |
|----------|-------------|
| `Renderer.setWindowIcon(path)` | Set window icon |
| `Renderer.getWidth()` | Framebuffer width (pixels) |
| `Renderer.getHeight()` | Framebuffer height |
| `Renderer.setLightDirection(x,y,z)` | Dir light direction |
| `Renderer.setLightColor(r,g,b)` | Dir light color |
| `Renderer.setAmbientColor(r,g,b)` | Ambient light color |
| `Renderer.setFogColor(r,g,b)` | Fog color |
| `Renderer.setFogDensity(d)` | Fog density |
| `Renderer.setFogStart(s)` | Fog start distance |
| `Renderer.setFogEnd(e)` | Fog end distance |
| `Renderer.setFogEnabled(bool)` | Toggle fog |
| `Renderer.setFlashlight(bool)` | Toggle flashlight |
| `Renderer.setFlashlightPos(x,y,z)` | Flashlight position |
| `Renderer.setFlashlightDir(x,y,z)` | Flashlight direction |
| `Renderer.setFlashlightColor(r,g,b)` | Flashlight color |
| `Renderer.setFlashlightCutoff(c)` | Flashlight cone angle |
| `Renderer.setRenderMode(bool)` | Wireframe mode |
| `Renderer.addPointLight(x,y,z, r,g,b, range)` | Add point light |
| `Renderer.removePointLight(id)` | Remove point light |

### Sprite

| Function | Description |
|----------|-------------|
| `Sprite.draw(texture, x, y, w, h)` | Draw sprite on overlay |
| `Sprite.drawRotated(texture, x, y, w, h, angle)` | Rotated sprite |
| `Sprite.drawColor(texture, x, y, w, h, r,g,b,a)` | Tinted sprite |
| `Sprite.drawString(text, x, y, scale, r,g,b,a)` | Draw text overlay |
| `Sprite.drawStringCentered(text, cx, y, scale, r,g,b,a)` | Centered text |
| `Sprite.measureWidth(text, scale)` | Measure text width |

### Audio

| Function | Description |
|----------|-------------|
| `Audio.loadSound(path)` | Load sound, returns handle |
| `Audio.playSound(h)` | Play sound once |
| `Audio.playSoundLoop(h)` | Play looping |
| `Audio.stopSound(h)` | Stop |
| `Audio.setVolume(h, vol)` | 0.0 – 1.0 |
| `Audio.setPitch(h, pitch)` | Playback speed |
| `Audio.setPosition(h, x,y,z)` | 3D position |
| `Audio.isPlaying(h)` | Is currently playing |
| `Audio.setMasterVolume(vol)` | Global volume |
| `Audio.setListenerPosition(x,y,z)` | Ear position |
| `Audio.setListenerOrientation(fx,fy,fz, ux,uy,uz)` | Ear direction |

### Physics

| Function | Description |
|----------|-------------|
| `Physics.raycast(ox,oy,oz, dx,dy,dz, maxDist)` | Ray intersection, returns `{hit, x,y,z, nx,ny,nz, entityId}` |
| `Physics.setGravity(x,y,z)` | Gravity vector |

### Map

| Function | Description |
|----------|-------------|
| `Map.load(path)` | Load a `.map` or `.lua` map file |
| `Map.createObject(templateName, x,y,z)` | Spawn from template |
| `Map.setTemplate(name, entityId)` | Register entity as template |

### Console

| Function | Description |
|----------|-------------|
| `Console.print(...)` | Print to game console |
| `Console.clear()` | Clear console |
| `Console.show(bool)` | Show/hide console |
| `Console.isOpen()` | Is console open |
| `Console.registerCommand(name, fn, help)` | Add custom command |
| `Console.registerVariable(name, getFn, setFn, help)` | Add custom variable |

### DEXECL

| Function | Description |
|----------|-------------|
| `Dexecl.execute(path)` | Execute a `.dexl` config file |
| `Dexecl.set(key, value)` | Set variable |
| `Dexecl.get(key)` | Get variable |
| `Dexecl.getEntity(name)` | Get entity ID created by dexecl |

### Time

| Function | Description |
|----------|-------------|
| `Time.delay(seconds, callback)` | Delayed one-shot |
| `Time.interval(seconds, callback)` | Repeated timer |
| `Time.clear(timerId)` | Cancel timer |

## Console Commands

| Command | Description |
|---------|-------------|
| `help` | List all commands |
| `clear` | Clear console |
| `quit` / `exit` | Exit application |
| `echo <text>` | Print text |
| `set <var> <value>` | Set console variable |
| `get <var>` | Get console variable |
| `list_vars` | Show all variables |
| `exec <path>` | Execute script |
| `lua <code>` | Run Lua one-liner |
| `load_map <path>` | Load a map |
| `reload` | Reload current map |
| `show_info` | Toggle FPS/entity overlay |
| `show_light` | Toggle light direction debug |
| `show_physics` | Toggle physics debug |
| `screenshot` | Take a screenshot |
| `fullscreen` | Toggle fullscreen |
| `vsync` | Toggle vsync |
| `fps <n>` | Set target FPS |
| `wireframe` | Toggle wireframe rendering |
| `post_effect <name>` | Set post-processing effect |
| `post_intensity <0-1>` | Effect intensity |
| `fog` | Toggle fog |
| `flashlight` | Toggle flashlight |
| `gravity <x,y,z>` | Set gravity |
| `audio_volume <0-1>` | Master volume |

Available post-effects: `none`, `vhs`, `dream`, `vignette`, `grayscale`, `invert`, `blur`, `pixelate`, `edgeDetect`, `sepia`, `bloom`, `crt`, `fishEye`, `motionBlur`, `sharpen`, `emboss`

## Map Format

Map files use a text format with `.map` extension. Each object is defined by one or more property lines:

```
[objectName;flags]:[MeshType](PROPERTY) = value
```

- **objectName** — name for template registration; use `_none` for unnamed scene-only objects
- **flags** — optional `key=value` pairs separated by `;`
- **MeshType** — mesh shape: `box`, `sphere`, `cylinder`, `plane`, or a path to a `.obj` file
- **PROPERTY** — property name
- **value** — property value

Lines with the same `objectName` define a single object. Named objects become templates callable via `Map.createObject()`.

### Example

```
[player]:box(POSITION) = 0,1,0
[player]:box(ROTATION) = 0,1,0,0
[player]:box(SCALE) = 0.5,0.5,0.5
[player]:box(COLOR) = 0.2,0.6,1.0,1
[player]:box(TEXTURE) = assets/player.png
[player]:box(COLLISION) = true
[player]:box(SCRIPT) = scripts/player.lua

[_none]:sphere(POSITION) = 3,0.5,2
[_none]:sphere(SCALE) = 0.3,0.3,0.3
[_none]:sphere(COLOR) = 1,0.3,0.1,1
[_none]:sphere(LIGHT_DIRECTIONAL) = true
```

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `POSITION` | `x,y,z` | World position |
| `ROTATION` | `x,y,z,w` | Quaternion rotation |
| `SCALE` | `x,y,z` | Scale |
| `COLOR` | `r,g,b,a` | Mesh color |
| `TEXTURE` | path | Texture file path |
| `SCRIPT` | path | Lua script |
| `COLLISION` | bool | Enable physics collision |
| `MASS` | float | Physics mass |
| `LIGHT_DIRECTIONAL` | bool | Directional light entity |
| `LIGHT_POINT` | bool | Point light entity |
| `LIGHT_SPOT` | bool | Spot light entity |
| `LIGHT_COLOR` | `r,g,b` | Light color |
| `LIGHT_RANGE` | float | Point light range |
| `PORTAL` | string | Portal target map |
| `FLAGS` | string | Generic flags |

### Legacy Lua Maps

`.lua` map files are still supported via `Map.load()` and use the old Lua table format.

## Configuration

The engine loads `manifest.plm` from the project directory. Example:

```lua
{
    name = "My Project",
    version = "1.0.0",
    main = "scripts/main.lua",
    assets = "assets/",
    libraries = { },
    build_output = "build/game",
    cross_compile_windows = false
}
```

Window and console configuration is handled via command-line arguments or Lua API, not the manifest file.

## Building

See [README.md](../README.md) for build instructions.

## Post-Processing Effects

Effects are stacked-single (only one active at a time) and controlled via console or Lua:

| Effect | Console Name | Description |
|--------|-------------|-------------|
| None | `none` | Direct blit (no effect) |
| VHS | `vhs` | Scanlines, jitter, chromatic aberration |
| Dream | `dream` | Blur + desaturation + vignette |
| Vignette | `vignette` | Darkened edges |
| Grayscale | `grayscale` | Desaturate |
| Invert | `invert` | Color inversion |
| Blur | `blur` | Gaussian blur |
| Pixelate | `pixelate` | Pixelation |
| Edge Detect | `edgeDetect` | Sobel edge detection |
| Sepia | `sepia` | Sepia tone |
| Bloom | `bloom` | Glow on bright areas |
| CRT | `crt` | Scanlines + distortion |
| FishEye | `fishEye` | Barrel distortion |
| Motion Blur | `motionBlur` | Directional blur |
| Sharpen | `sharpen` | Sharpening kernel |
| Emboss | `emboss` | Emboss filter |
