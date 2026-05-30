# Planet Engine API Reference

> **Planet Engine** — a Lua-scriptable, real-time 3D/2D game engine powered by OpenGL 4.1, GLFW, GLM, Lua 5.4, and OpenAL/FMOD.

---

## Table of Contents

1. [Core](#1-core)
   - [EngineConfig](#engineconfig)
   - [Engine](#engine)
   - [Window](#window)
   - [Input / KeyCode / MouseButton](#input)
   - [Camera / Ray / ProjectionType](#camera)
   - [Scene](#scene)
   - [Console / LogLevel / LogEntry / ConsoleCommand](#console)
   - [Logger / LogLine](#logger)
   - [DexeclEngine](#dexeclengine)
   - [TimerManager / Timer / FrameTimer](#timers)
   - [Random](#random)
   - [Math](#math-namespace)
2. [Render](#2-render)
   - [Renderer / PointLight](#renderer)
   - [Shader](#shader)
   - [Texture](#texture)
   - [Vertex / Mesh](#mesh)
   - [Framebuffer](#framebuffer)
   - [ModelRenderer / RenderCommand](#modelrenderer)
   - [SpriteRenderer](#spriterenderer)
   - [TextRenderer / GlyphInfo](#textrenderer)
   - [PostProcessor / PostEffect](#postprocessor)
   - [Portal](#portal-struct)
   - [PortalManager / TeleportResult](#portalmanager)
3. [ECS](#3-ecs)
   - [Entity](#entity)
   - [Component](#component)
   - [Built-in Components](#built-in-components)
   - [System / SystemManager](#systems)
4. [Physics](#4-physics)
   - [Physics / RaycastHit / OverlapResult](#physics)
5. [Audio](#5-audio)
   - [Audio (OpenAL)](#audio-openal)
   - [FmodAudio](#fmodaudio)
   - [Audio Stub](#audio-stub)
6. [Debug](#6-debug)
   - [DebugServer](#debugserver)
7. [Resource](#7-resource)
   - [ResourceManager](#resourcemanager)
8. [Lua Bindings](#8-lua-bindings)
   - [Engine](#lua-engine-table)
   - [Camera](#lua-camera-table)
   - [Scene](#lua-scene-table)
   - [Console](#lua-console-table)
   - [Renderer](#lua-renderer-table)
   - [Input](#lua-input-table)
   - [ECS](#lua-ecs-table)
   - [Physics](#lua-physics-table)
   - [Audio](#lua-audio-table)
   - [Portal](#lua-portal-table)
   - [Dexecl](#lua-dexecl-table)
   - [Math](#lua-math-table)
   - [Random](#lua-random-table)
   - [Timer](#lua-timer-table)
9. [Platform Support Notes](#9-platform-support-notes)

---

## 1. Core

### EngineConfig

| Field | Type | Default | Description |
|---|---|---|---|
| `windowWidth` | `int` | `1280` | Initial window width (px) |
| `windowHeight` | `int` | `720` | Initial window height (px) |
| `windowTitle` | `std::string` | `"Planet"` | Window title |
| `fullscreen` | `bool` | `false` | Start in fullscreen mode |
| `vsync` | `bool` | `false` | Enable vertical sync |
| `targetFPS` | `int` | `1000` | Target FPS cap (0 = uncapped) |
| `scriptPath` | `std::string` | `"scripts/main.lua"` | Main Lua script path |
| `assetPath` | `std::string` | `"assets/"` | Asset root directory |
| `kerdataPath` | `std::string` | *(empty)* | Path to .kerdata archive |
| `termOutput` | `bool` | `false` | Enable terminal log output |
| `consoleEnabled` | `bool` | `false` | Enable in-game console |

### Engine

Singleton class `planet::Engine`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Engine&` | Get singleton instance |
| `Init(const EngineConfig&)` | `bool` | Initialize engine subsystems |
| `Run()` | `void` | Start the main loop |
| `Shutdown()` | `void` | Clean shutdown of all subsystems |
| `IsRunning()` | `bool` | Check if engine is running |
| `GetDeltaTime()` | `double` | Frame delta time in seconds |
| `GetElapsedTime()` | `double` | Total elapsed time since start |
| `GetConfig()` | `const EngineConfig&` | Engine configuration |
| `Quit()` | `void` | Request graceful exit |
| `GetFrameCount()` | `uint64_t` | Total frames rendered |
| `GetAverageFPS()` | `double` | Smoothed frames-per-second |
| `GetTimeSinceStart()` | `double` | Wall clock time via `glfwGetTime()` |
| `SetTargetFPS(int)` | `void` | Change FPS cap at runtime |
| `GetTargetFPS()` | `int` | Current FPS cap |
| `SetVSync(bool)` | `void` | Toggle vsync at runtime |
| `IsVSync()` | `bool` | Check vsync status |
| `SetFullscreen(bool)` | `void` | Toggle fullscreen at runtime |
| `IsFullscreen()` | `bool` | Check fullscreen status |

**Configuration from command line:**
| Flag | Effect |
|---|---|
| `--term` / `-t` | Enable terminal output |
| `--console` / `-c` | Enable in-game console |
| `*.kerdata` | Load kerdata archive |
| `*.lua` | Use as main script |

---

### Window

Singleton class `planet::Window`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Window&` | Get singleton |
| `Create(w, h, title, fullscreen?)` | `bool` | Create window + OpenGL context |
| `Destroy()` | `void` | Destroy window, terminate GLFW |
| `ShouldClose()` | `bool` | Check for close request |
| `SwapBuffers()` | `void` | Swap front/back buffers |
| `GetGLFWWindow()` | `GLFWwindow*` | Raw GLFW window handle |
| `GetWidth()` | `int` | Framebuffer width |
| `GetHeight()` | `int` | Framebuffer height |
| `GetAspectRatio()` | `float` | Width / height |
| `SetTitle(const string&)` | `void` | Set window title |
| `SetIcon(const string&)` | `void` | Set window icon from PNG |
| `SetVSync(bool)` | `void` | Toggle vsync |
| `IsVSync()` | `bool` | Check vsync |
| `SetSize(w, h)` | `void` | Resize window |
| `SetFullscreen(bool)` | `void` | Toggle fullscreen |
| `IsFullscreen()` | `bool` | Check fullscreen |
| `Minimize()` | `void` | Iconify window |
| `Restore()` | `void` | Restore from minimized |
| `Maximize()` | `void` | Maximize window |
| `Focus()` | `void` | Request input focus |
| `IsFocused()` | `bool` | Check if focused |
| `IsMinimized()` | `bool` | Check if iconified |
| `SetOpacity(float)` | `void` | Set window opacity [0-1] |
| `SetSizeLimits(minW, minH, maxW, maxH)` | `void` | Constrain resize range |

**Callbacks:**
| Static Callback | Purpose |
|---|---|
| `FramebufferSizeCallback(GLFWwindow*, w, h)` | Viewport resize |
| `WindowFocusCallback(GLFWwindow*, focused)` | Focus change event |

---

### Input

Singleton class `planet::Input`.

**Key Getter Methods:**
| Method | Returns | Description |
|---|---|---|
| `GetKey(KeyCode)` | `bool` | Is key held this frame? |
| `GetKeyDown(KeyCode)` | `bool` | Was key pressed this frame? |
| `GetKeyUp(KeyCode)` | `bool` | Was key released this frame? |
| `GetAnyKeyDown()` | `bool` | Was any key pressed this frame? |
| `GetAnyKey()` | `bool` | Is any key held? |
| `GetKeyName(KeyCode)` | `string` | Printable key name |

**Mouse Methods:**
| Method | Returns | Description |
|---|---|---|
| `GetMouseButton(MouseButton)` | `bool` | Is button held? |
| `GetMouseButtonDown(MouseButton)` | `bool` | Was button pressed? |
| `GetMouseButtonUp(MouseButton)` | `bool` | Was button released? |
| `GetAnyMouseButtonDown()` | `bool` | Was any mouse button pressed? |
| `GetMousePosition()` | `glm::vec2` | Current cursor position (px) |
| `GetMouseDelta()` | `glm::vec2` | Cursor movement since last frame |
| `GetScrollDelta()` | `float` | Scroll wheel delta |
| `SetMouseLocked(bool)` | `void` | Lock/unlock cursor |
| `IsMouseLocked()` | `bool` | Check mouse lock state |

**Text Input:**
| Method | Returns | Description |
|---|---|---|
| `GetTypedChars()` | `vector<unsigned int>` | Unicode codepoints typed this frame |
| `ClearTypedChars()` | `void` | Clear typed char buffer |

**KeyCode enum class** maps to GLFW key constants:

Categories:
- **Letters:** `A`-`Z`
- **Digits:** `Num0`-`Num9`
- **Modifiers:** `LeftShift`, `RightShift`, `LeftControl`, `RightControl`, `LeftAlt`, `RightAlt`, `LeftSuper`, `RightSuper`
- **Navigation:** `Up`, `Down`, `Left`, `Right`, `Home`, `End`, `PageUp`, `PageDown`
- **Action:** `Space`, `Enter`, `Escape`, `Tab`, `Backspace`, `Insert`, `Delete`
- **Symbols:** `Apostrophe`, `Comma`, `Minus`, `Period`, `Slash`, `Semicolon`, `Equal`, `LeftBracket`, `Backslash`, `RightBracket`, `GraveAccent`
- **Lock:** `CapsLock`, `ScrollLock`, `NumLock`
- **Other:** `PrintScreen`, `Pause`, `F1`-`F12`

**MouseButton enum class:**
| Value | Name |
|---|---|
| `0` | `Left` |
| `1` | `Right` |
| `2` | `Middle` |

---

### Camera

Class `planet::Camera` with `ProjectionType` enum (`Perspective`, `Orthographic`).

| Method | Returns | Description |
|---|---|---|
| `SetProjectionType(ProjectionType)` | `void` | Perspective or Orthographic |
| `GetProjectionType()` | `ProjectionType` | Current projection mode |
| `SetPosition(glm::vec3)` | `void` | Camera world position |
| `GetPosition()` | `glm::vec3` | Camera position |
| `SetTarget(glm::vec3)` | `void` | Look-at target |
| `GetTarget()` | `glm::vec3` | Look-at target |
| `SetUp(glm::vec3)` | `void` | Up vector |
| `SetFOV(float)` | `void` | Field of view in degrees |
| `GetFOV()` | `float` | Current FOV |
| `SetNearPlane(float)` | `void` | Near clip plane |
| `GetNearPlane()` | `float` | Near clip distance |
| `SetFarPlane(float)` | `void` | Far clip plane |
| `GetFarPlane()` | `float` | Far clip distance |
| `SetOrthoSize(float)` | `void` | Orthographic half-height |
| `GetOrthoSize()` | `float` | Ortho size |
| `GetViewMatrix()` | `glm::mat4` | View matrix (cached, dirty-checked) |
| `GetProjectionMatrix(float aspect)` | `glm::mat4` | Projection matrix |
| `Forward()` | `glm::vec3` | Forward direction (target - position) |
| `Right()` | `glm::vec3` | Right direction |
| `Up()` | `glm::vec3` | Up direction |
| `SetRotation(glm::quat)` | `void` | Set orientation via quaternion |
| `GetRotation()` | `glm::quat` | Get current orientation |
| `Orbit(float yawDeg, float pitchDeg)` | `void` | Orbit camera around target |
| `LookAt(glm::vec3)` | `void` | Point camera at world position |
| `LookAtEntity(Entity*)` | `void` | Point camera at entity |
| `SetPitch(float)` | `void` | Set pitch angle (clamped ±89°) |
| `SetYaw(float)` | `void` | Set yaw angle |
| `GetPitch()` | `float` | Current pitch |
| `GetYaw()` | `float` | Current yaw |
| `SetZoom(float)` | `void` | Zoom factor [0.1, 100] |
| `GetZoom()` | `float` | Current zoom |
| `ScreenToWorldPoint(sx, sy, sz, vw, vh)` | `glm::vec3` | Screen → world coordinates |
| `WorldToScreenPoint(worldPos, vw, vh)` | `glm::vec2` | World → screen coordinates |
| `ScreenToRay(sx, sy, vw, vh)` | `Ray` | Create ray from screen point |
| `GetFrustumWidthAtDistance(dist, aspect)` | `float` | Frustum width at distance |
| `GetFrustumHeightAtDistance(dist)` | `float` | Frustum height at distance |

**Default values:** `pos(0,2,5)`, `target(0,0,0)`, `up(0,1,0)`, `FOV 60°`, `near 0.1`, `far 1000`, `orthoSize 10`, `zoom 1`, `yaw -90°`, `pitch 0°`.

**Ray struct:**
| Field | Type | Default |
|---|---|---|
| `origin` | `glm::vec3` | `(0,0,0)` |
| `direction` | `glm::vec3` | `(0,0,-1)` |

---

### Scene

Singleton class `planet::Scene`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Scene&` | Get singleton |
| `Load()` | `void` | Create default camera |
| `Unload()` | `void` | Clear all entities and camera |
| `Update(double dt)` | `void` | Update all entities |
| `CreateEntity(const string&)` | `Entity*` | Create named entity |
| `CreateEntityFromPrefab(const string&)` | `Entity*` | *(not implemented)* |
| `DestroyEntity(Entity*)` | `void` | Destroy entity |
| `FindEntity(const string&)` | `Entity*` | Find by name (first match) |
| `FindEntitiesByName(const string&)` | `vector<Entity*>` | All entities matching name |
| `GetActiveCamera()` | `Camera*` | Active camera |
| `SetActiveCamera(unique_ptr<Camera>)` | `void` | Set active camera |
| `Clear()` | `void` | Remove all entities |
| `GetEntities()` | `const vector<Entity*>&` | Entity list |
| `GetEntitiesWithTag(const string&)` | `vector<Entity*>` | Entities with tag |
| `GetEntityCount()` | `size_t` | Number of entities |
| `DestroyAllWithTag(const string&)` | `void` | Remove all entities with tag |
| `GetEntityAtIndex(size_t)` | `Entity*` | Entity by index |
| `SetAmbientLight(glm::vec3)` | `void` | Ambient light color |
| `GetAmbientLight()` | `glm::vec3` | Current ambient light |

---

### Console

Singleton class `planet::Console`.

| Method | Returns | Description |
|---|---|---|
| `Init()` / `Shutdown()` | `void` | Lifecycle |
| `Update(double)` / `Render()` | `void` | Update and render console UI |
| `Toggle()` | `void` | Open/close toggle |
| `IsOpen()` | `bool` | Is console visible? |
| `SetOpen(bool)` | `void` | Force open/close |
| `IsEnabled()` / `SetEnabled(bool)` | `bool` / `void` | Global enable/disable |
| `AddChar(unsigned int)` | `void` | Input character |
| `AddCommand(ConsoleCommand)` | `void` | Register command |
| `Execute(const string&)` | `void` | Execute command line |
| `Print(const string&)` | `void` | Log message |
| `Print(LogLevel, const string&)` | `void` | Log with level |
| `PrintError/Warning/Debug` | `void` | Level-specific logging |
| `RaycastEntity()` | `Entity*` | Crosshair entity pick |
| `CheatsEnabled()` / `SetCheatsEnabled(bool)` | `bool` / `void` | Cheat mode |

**Public state flags:**
| Field | Type | Description |
|---|---|---|
| `showLightVector` | `bool` | Debug light direction overlay |
| `showInstanceInfo` | `bool` | Debug entity info overlay |
| `noclipEnabled` | `bool` | Noclip camera mode |

**Logging levels:** `Info`, `Warning`, `Error`, `Debug`.

**ConsoleCommand fields:** `name`, `prefix` ("do"/"sr"/"cr"), `args`, `desc`, `handler` (function).

**Built-in commands:**
- `clear` — clear log
- `help` — list commands
- `quit` — exit engine
- `noclip` — toggle noclip
- `setpos x y z` — teleport
- `lv` — toggle light vector
- `ii` — toggle instance info
- `cheats` — toggle cheats
- `exec <file>` — run Dexecl script

---

### Logger

Inline utilities in `planet` namespace.

| Global | Description |
|---|---|
| `g_termEnabled` | Global terminal output toggle |
| `Timestamp()` | Returns `HH:MM:SS.mmm` |
| `LogLine(bool error)` | Stream-based logger |
| `LOG_INFO()` | Creates `LogLine(false)` |
| `LOG_ERROR()` | Creates `LogLine(true)` |

Usage:
```cpp
LOG_INFO() << "[Tag] Message " << value;
LOG_ERROR() << "[Tag] Error: " << reason;
```

---

### DexeclEngine

Singleton class `planet::DexeclEngine`. A minimal imperative scripting engine with variables, `if/endif`, `for/next`, `while/wend`, math expressions, and built-in commands.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `DexeclEngine&` | Singleton |
| `Clear()` | `void` | Clear program state |
| `ExecuteFile(const string&)` | `bool` | Load and execute `.dexl` file |
| `ExecuteString(const string&)` | `void` | Execute inline code |
| `ExecuteLine(const string&)` | `void` | Execute single line |
| `SetVariable(name, value)` | `void` | Set float variable |
| `GetVariable(name, default?)` | `float` | Get float variable |
| `DeleteVariable(name)` | `void` | Remove variable |
| `HasVariable(name)` | `bool` | Check existence |
| `GetVariables()` | `const map<string,float>&` | All variables |

**Internal states:** `Normal`, `IfSkip`, `ForLoop`, `WhileLoop`.

---

### TimerManager / Timer / FrameTimer

Class `planet::TimerManager` (singleton).

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `TimerManager&` | Singleton |
| `AddTimer(interval, callback, looping?)` | `int` | Create timer, returns ID |
| `RemoveTimer(int id)` | `void` | Remove timer by ID |
| `ClearAll()` | `void` | Remove all timers |
| `Update(double dt)` | `void` | Advance all active timers |
| `IsTimerActive(int id)` | `bool` | Check if timer exists and active |
| `AddDelayedCall(delay, callback)` | `int` | One-shot delayed callback |
| `PauseTimer(int id)` | `void` | Pause without removing |
| `ResumeTimer(int id)` | `void` | Resume paused timer |

**Timer struct:**
| Field | Type | Default |
|---|---|---|
| `interval` | `double` | `1.0` |
| `elapsed` | `double` | `0.0` |
| `looping` | `bool` | `true` |
| `active` | `bool` | `true` |
| `callback` | `function<void()>` | — |
| `id` | `int` | `0` |
| `oneshot` | `bool` | `false` |

**FrameTimer struct:**
| Method / Field | Description |
|---|---|
| `Tick(double dt)` | Accumulate and compute FPS |
| `Reset()` | Reset all counters |
| `frameCount` | Frames since last reset |
| `elapsed` | Time since last reset |
| `fps` | Smoothed FPS |
| `minFps` | Minimum observed FPS |
| `maxFps` | Maximum observed FPS |

---

### Random

Class `planet::Random` (singleton).

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Random&` | Singleton |
| `SetSeed(uint64_t)` | `void` | Seed the RNG |
| `GetSeed()` | `uint64_t` | Current seed |
| `Range(min, max)` (float) | `float` | Uniform float |
| `Range(min, max)` (int) | `int` | Uniform int |
| `Range(min, max)` (double) | `double` | Uniform double |
| `Value()` | `float` | [0, 1) float |
| `Sign()` | `float` | -1 or +1 |
| `Vec3(min, max)` | `glm::vec3` | Random vec3 |
| `UnitVector()` | `glm::vec3` | Random unit vector |
| `InsideUnitSphere()` | `glm::vec3` | Random point in sphere |
| `InsideUnitCube()` | `glm::vec3` | Random point in cube |
| `Vec2(min, max)` | `glm::vec2` | Random vec2 |
| `InsideUnitCircle()` | `glm::vec2` | Random point in circle |
| `Gaussian(mean, stddev)` | `float` | Normal distribution |
| `Choice(vector)` | `T` | Random element |
| `Shuffle(vector<int>&)` | `void` | Fisher-Yates shuffle |

Default seed: `12345`.

---

### Math Namespace

`planet::Math` — GLSL-style math utilities.

**Constants:**
| Name | Value |
|---|---|
| `PI` | `3.1415927` |
| `TWO_PI` | `6.2831855` |
| `HALF_PI` | `1.5707963` |
| `DEG2RAD` | `PI/180` |
| `RAD2DEG` | `180/PI` |
| `EPSILON` | `1e-6f` |

**Interpolation:**
| Function | Description |
|---|---|
| `Lerp(a, b, t)` | Linear interpolation (float/vec3/vec4) |
| `Slerp(quat, quat, t)` | Spherical linear interpolation |
| `SmoothStep(edge0, edge1, x)` | Hermite interpolation |
| `SmootherStep(edge0, edge1, x)` | 5th-order interpolation |

**Easing Functions:**
| Category | Functions |
|---|---|
| Quad | `EaseInQuad`, `EaseOutQuad`, `EaseInOutQuad` |
| Cubic | `EaseInCubic`, `EaseOutCubic`, `EaseInOutCubic` |
| Elastic | `EaseInElastic`, `EaseOutElastic` |
| Bounce | `EaseInBounce`, `EaseOutBounce` |

**Movement:**
| Function | Description |
|---|---|
| `Clamp(value, min, max)` | Clamp with 3 overloads (float/int/double/vec3) |
| `Map(value, inMin, inMax, outMin, outMax)` | Remap range |
| `Repeat(t, length)` | Looping value |
| `PingPong(t, length)` | Oscillating value |
| `MoveTowards(cur, target, maxDelta)` | Constant-speed approach |
| `MoveTowardsAngle(cur, target, maxDelta)` | Angle version using `DeltaAngle` |
| `DeltaAngle(cur, target)` | Signed angle difference |
| `SmoothDamp(cur, target, velocity, time, maxSpeed, dt)` | Critically damped spring (float/vec3) |

**LookAt:**
| Function | Description |
|---|---|
| `LookAt(from, to)` | Quaternion rotation from->to |

**Geometry:**
| Function | Description |
|---|---|
| `AngleBetween(a, b)` | Angle between two vectors (radians) |
| `DistancePointToLine(point, start, end)` | Closest point on segment |
| `DistancePointToPlane(point, normal, d)` | Perpendicular distance |
| `ProjectOnPlane(vector, normal)` | Vector projection onto plane |
| `Reflect(direction, normal)` | Reflection vector |
| `Refract(direction, normal, eta)` | Refraction vector |
| `RandomOnUnitSphere()` | Random point on sphere surface |
| `RandomInUnitHemisphere(normal)` | Random point in hemisphere |

**Intersection Tests:**
| Function | Description |
|---|---|
| `RayIntersectsSphere(origin, dir, center, radius, t)` | Ray-sphere test |
| `RayIntersectsAABB(origin, dir, min, max, t)` | Ray-AABB test (slabs) |

**Utilities:**
| Function | Description |
|---|---|
| `IsPowerOfTwo(int)` | Power-of-two check |
| `NextPowerOfTwo(int)` | Round up to nearest POT |

---

## 2. Render

### Renderer

Singleton class `planet::Renderer`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Renderer&` | Singleton |
| `Init(w, h)` / `Shutdown()` | `void` | Lifecycle |
| `BeginFrame()` / `EndFrame()` | `void` | Frame boundaries |
| `SetViewMatrix(mat4)` | `void` | View matrix |
| `SetProjectionMatrix(mat4)` | `void` | Projection matrix |
| `GetViewMatrix()` | `const mat4&` | Current view |
| `GetProjectionMatrix()` | `const mat4&` | Current projection |
| `PushMatrix(mat4)` / `PopMatrix()` | `void` | Matrix stack |
| `GetTopMatrix()` | `const mat4&` | Stack top |
| `GetDefaultShader()` | `Shader*` | Default 3D shader |
| `GetSpriteShader()` | `Shader*` | Sprite shader |
| `ClearColor(r,g,b,a)` | `void` | Set clear color |
| `GetClearColor(r,g,b,a)` | `void` | Get clear color |
| `SetRenderMode(bool wireframe)` | `void` | Wireframe toggle |
| `IsWireframe()` | `bool` | Wireframe state |

**Directional Light:**
| Method | Description |
|---|---|
| `SetLightDirection(vec3)` | Normalized direction |
| `SetLightColor(vec3)` | RGB color |
| `SetAmbientColor(vec3)` | Ambient light |
| `GetLightDirection/Color/Ambient` | Getters |

**Fog:**
| Method | Description |
|---|---|
| `SetFogColor(vec3)` | Fog tint |
| `SetFogDensity(float)` | Exponential density |
| `SetFogStart/End(float)` | Linear fog range |
| `SetFogEnabled(bool)` | Toggle |
| `IsFogEnabled()` | State check |

**Flashlight:**
| Method | Description |
|---|---|
| `SetFlashlightEnabled(bool)` | Toggle |
| `SetFlashlightPos/Dir/Color/Cutoff` | Configure |
| Getters for all properties | Query state |

**Point Lights:**
`static const int MAX_POINT_LIGHTS = 16`

| Method | Returns | Description |
|---|---|---|
| `AddPointLight(pos, color, range)` | `int` | Add light, returns index |
| `RemovePointLight(index)` | `void` | Remove by index |
| `ClearPointLights()` | `void` | Remove all |
| `SetPointLightPosition/Color(index, ...)` | `void` | Update |
| `GetPointLightCount()` | `int` | Number of active lights |
| `GetPointLights()` | `const PointLight*` | Raw array |

**PointLight struct:**
| Field | Type | Default |
|---|---|---|
| `position` | `glm::vec3` | `(0,0,0)` |
| `color` | `glm::vec3` | `(1,1,1)` |
| `range` | `float` | `5.0` |

---

### Shader

Class `planet::Shader`.

| Method | Returns | Description |
|---|---|---|
| `LoadFromFile(vertPath, fragPath)` | `bool` | Load from files |
| `LoadFromSource(vertSrc, fragSrc)` | `bool` | Load from source strings |
| `Bind()` / `Unbind()` | `void` | Activate/deactivate |
| `GetID()` | `GLuint` | OpenGL program handle |
| `SetInt(name, value)` | `void` | Integer uniform |
| `SetFloat(name, value)` | `void` | Float uniform |
| `SetVec2/3/4(name, value)` | `void` | Vector uniforms |
| `SetMat4(name, value)` | `void` | Matrix uniform |
| `SetBool(name, value)` | `void` | Boolean (as int) |
| `Reload()` | `bool` | Hot-reload from source |
| `IsValid()` | `bool` | Program loaded and linked |
| `HasUniform(name)` | `bool` | Check uniform existence |
| `GetVertexPath()` / `GetFragmentPath()` | `string` | Source file paths |

**Default vertex shader** — attribs: `aPos(Vec3)`, `aNormal(Vec3)`, `aTexCoord(Vec2)`, `aColor(Vec4)`. Uniforms: `uModel`, `uView`, `uProjection`.

**Default fragment shader** — supports: texture + vertex color, directional light, point lights (up to 16), flashlight, exponential fog.

**Sprite shader** — 2D position, texture, color.

---

### Texture

Class `planet::Texture`.

| Method | Returns | Description |
|---|---|---|
| `LoadFromFile(path)` | `bool` | Load image via stb_image |
| `CreateEmpty(w, h, format?)` | `bool` | Blank texture |
| `Bind(GLuint slot?)` | `void` | Bind to texture unit |
| `Unbind()` | `void` | Unbind |
| `GetID()` | `GLuint` | OpenGL texture handle |
| `GetWidth()` / `GetHeight()` | `int` | Dimensions |
| `GetInternalFormat()` | `GLenum` | Internal format |
| `GetFormat()` | `GLenum` | Pixel format |
| `IsValid()` | `bool` | Has valid ID? |
| `SetFilterMode(min, mag)` | `void` | Filter settings |
| `SetWrapMode(s, t)` | `void` | Wrap settings |
| `SaveToFile(path)` | `bool` | *(not implemented)* |
| `GetPixel(x, y)` | `glm::vec4` | *(not directly supported)* |
| `SetPixel(x, y, color)` | `void` | *(not directly supported)* |
| `Resize(w, h)` | `void` | Reallocate |
| `Clear(color)` | `void` | *(not directly supported)* |
| `SetData(data, w, h, format?)` | `void` | Upload raw pixel data |
| `GenerateMipmaps()` | `void` | Build mip chain |

---

### Vertex / Mesh

**Vertex struct:**
| Field | Type | Default |
|---|---|---|
| `Position` | `glm::vec3` | — |
| `Normal` | `glm::vec3` | — |
| `TexCoords` | `glm::vec2` | — |
| `Color` | `glm::vec4` | `(1,1,1,1)` |

**Mesh class:**

| Method / Factory | Returns | Description |
|---|---|---|
| `Mesh()` | — | Default constructor |
| `Mesh(vertices, indices)` | — | Construct + upload |
| `Upload(vertices, indices)` | `void` | Upload to GPU |
| `Draw()` | `void` | Render indexed triangles |
| `Destroy()` | `void` | Free GPU resources |
| `GetVAO()` | `GLuint` | Vertex array object |
| `GetIndexCount()` | `size_t` | Number of indices |
| `GetVertexCount()` | `size_t` | Number of vertices |

**Factory Methods (all static):**
| Method | Args |
|---|---|
| `CreateQuad(w, h)` | Width, height |
| `CreateCube(size)` | Size |
| `CreateSphere(radius, segments)` | Radius, subdivisions (default 16) |
| `CreatePlane(w, d)` | Width, depth |
| `CreateBox(w, h, d)` | Width, height, depth |
| `CreateWallQuad(w, h)` | Double-sided vertical quad |
| `CreateCylinder(r, h, segs)` | Radius, height, segments |
| `CreateCone(r, h, segs)` | Radius, height, segments |
| `CreateTorus(r, tubeR, segs, tubeSegs)` | Major radius, tube radius, subdivisions |
| `CreateIcosahedron(radius)` | Radius |
| `CreateGrid(w, d, divisions)` | Width, depth, divisions |
| `CreateCircle(r, segs)` | Radius, segments |
| `CreateTetrahedron(radius)` | Radius |
| `CreateOctahedron(radius)` | Radius |

---

### Framebuffer

Class `planet::Framebuffer`.

| Method | Returns | Description |
|---|---|---|
| `Init(w, h)` | `bool` | Create FBO with color + depth |
| `Resize(w, h)` | `void` | Reallocate |
| `Destroy()` | `void` | Free GPU resources |
| `Bind()` / `Unbind()` | `void` | Activate/deactivate |
| `GetColorTexture()` | `GLuint` | Color attachment texture |
| `GetFboId()` | `GLuint` | OpenGL FBO handle |
| `GetWidth()` / `GetHeight()` | `int` | Dimensions |

---

### ModelRenderer

Singleton class `planet::ModelRenderer`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `ModelRenderer&` | Singleton |
| `Init()` / `Shutdown()` | `void` | Lifecycle |
| `SubmitMesh(mesh, tex, transform, color?, unlit?)` | `void` | Queue mesh for rendering |
| `SubmitWireframe(mesh, transform, color?)` | `void` | Queue wireframe overlay |
| `BeginFrame()` / `EndFrame()` | `void` | Frame boundaries |
| `Flush()` | `void` | Force immediate draw |

**RenderCommand struct:**
| Field | Type | Description |
|---|---|---|
| `mesh` | `Mesh*` | Mesh to render |
| `texture` | `Texture*` | Optional texture |
| `transform` | `glm::mat4` | Model matrix |
| `color` | `glm::vec4` | Tint color |
| `unlit` | `bool` | Skip lighting |

---

### SpriteRenderer

Singleton class `planet::SpriteRenderer`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `SpriteRenderer&` | Singleton |
| `Init()` / `Shutdown()` | `void` | Lifecycle |
| `DrawSprite(tex, pos, size, rotation?, tint?, pivot?)` | `void` | Textured sprite |
| `DrawSprite(pos, size, color, rotation?)` | `void` | Colored quad |
| `BeginBatch()` / `EndBatch()` | `void` | Batch boundaries |
| `Flush()` | `void` | Flush batched geometry |

---

### TextRenderer

Singleton class `planet::TextRenderer`.

| Constants | Value |
|---|---|
| `NUM_GLYPHS` | `95` |
| `FIRST_CHAR` | `32` (space) |
| `FONT_HEIGHT` | `24.0` |
| `ATLAS_W` / `ATLAS_H` | `512` |
| `GetCharWidth()` | `8.0px` |
| `GetCharHeight()` | `16.0px` |

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `TextRenderer&` | Singleton |
| `Init()` / `Shutdown()` | `void` | Lifecycle |
| `DrawString(text, x, y, scale?, color?)` | `void` | Render text |
| `DrawStringCentered(text, centerX, y, scale?, color?)` | `void` | Centered text |

**GlyphInfo struct:**
| Field | Type | Description |
|---|---|---|
| `u0, v0, u1, v1` | `float` | Atlas UV coordinates |
| `xoff, yoff` | `float` | Bearing offsets |
| `advance` | `float` | Horizontal advance |

---

### PostProcessor

Singleton class `planet::PostProcessor`.

**PostEffect enum:**
`None`, `VHS`, `Dream`, `Vignette`, `Grayscale`, `Invert`, `Blur`, `Pixelate`, `EdgeDetect`, `Sepia`, `Bloom`, `CRT`, `FishEye`, `MotionBlur`, `Sharpen`, `Emboss`

(*Only VHS, Dream, Vignette have implemented shaders; others fall back to None.*)

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `PostProcessor&` | Singleton |
| `Init(w, h)` / `Shutdown()` | `void` | Lifecycle |
| `Resize(w, h)` | `void` | Handle resize |
| `BeginCapture()` | `void` | Render scene to FBO |
| `EndCapture()` | `void` | Apply effect and blit |
| `SetEffect(PostEffect)` | `void` | Set active effect |
| `GetEffect()` | `PostEffect` | Current effect |
| `SetEffectByName(string)` | `void` | Set by name string |
| `SetEffectIntensity(float)` | `void` | Effect intensity [0-1+] |
| `GetEffectIntensity()` | `float` | Current intensity |
| `GetTime()` / `SetTime(float)` | — | Time uniform for animated effects |
| `GetAvailableEffectNames()` | `vector<string>` | All effect names |
| `GetEffectName(PostEffect)` | `string` | Convert enum to name |
| `GetEffectFromName(string)` | `PostEffect` | Convert name to enum |
| `SetEffectParameter(name, value)` | `void` | Generic parameter setter |
| `GetEffectParameter(name)` | `float` | Generic parameter getter |

---

### Portal (struct)

`planet::Portal`

| Field | Type | Default |
|---|---|---|
| `id` | `int` | `-1` |
| `name` | `string` | — |
| `position` | `glm::vec3` | `(0,0,0)` |
| `rotation` | `glm::quat` | Identity |
| `size` | `glm::vec2` | `(2,3)` |
| `linkedPortalId` | `int` | `-1` |
| `active` | `bool` | `true` |
| `color` | `glm::vec4` | `(0.3, 0.7, 1.0, 0.6)` |

| Method | Returns | Description |
|---|---|---|
| `GetModelMatrix()` | `glm::mat4` | TRS model matrix |
| `GetForward()` | `glm::vec3` | Portal forward direction |
| `GetUp()` | `glm::vec3` | Portal up direction |
| `IsPointInFront(point)` | `bool` | Point in front of portal? |
| `IsPointCrossing(oldPos, newPos)` | `bool` | Did point cross portal plane? |
| `ComputeWarpPosition(dst, crossingPos)` | `glm::vec3` | Warp position through linked portal |
| `ComputeWarpDirection(dst)` | `glm::vec3` | Warp forward direction |

---

### PortalManager

Singleton class `planet::PortalManager`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `PortalManager&` | Singleton |
| `Init(w, h)` / `Shutdown()` | `void` | Lifecycle |
| `Resize(w, h)` | `void` | Handle resize |
| `CreatePortal(name, pos, rot, size)` | `int` | Create portal, returns ID |
| `GetPortal(id)` | `Portal*` | Get by ID |
| `GetPortalByName(name)` | `Portal*` | Find by name |
| `LinkPortals(a, b)` | `void` | Link two portals |
| `SetPortalColor(id, color)` | `void` | Set tint |
| `SetPortalActive(id, bool)` | `void` | Enable/disable |
| `SetPortalPosition(id, pos)` | `void` | Move portal |
| `DestroyPortal(id)` | `void` | Remove portal |
| `ClearPortals()` | `void` | Remove all |
| `SetRecursionDepth(depth)` | `void` | Recursion [0-4] |
| `GetRecursionDepth()` | `int` | Current depth |
| `RenderPortals(view, proj, camPos, callback)` | `void` | Render recursive portal views |
| `CheckTeleport(oldPos, newPos)` | `TeleportResult` | Teleport check |
| `GetPortals()` | `vector<Portal>` | All portals |

**TeleportResult struct:**
| Field | Type | Description |
|---|---|---|
| `occurred` | `bool` | Teleport happened? |
| `sourcePortalId` | `int` | Which portal was crossed |
| `warpPosition` | `glm::vec3` | New position |
| `warpForward` | `glm::vec3` | New forward direction |

---

## 3. ECS

### Entity

Class `planet::Entity`.

| Method | Returns | Description |
|---|---|---|
| `Entity(name?)` | — | Constructor |
| `Update(dt)` / `Render()` | `void` | Update components |
| `GetName()` / `SetName(name)` | `string` / `void` | Entity name |
| `AddTag(tag)` / `RemoveTag(tag)` | `void` | Tag management |
| `HasTag(tag)` | `bool` | Tag check |
| `GetTags()` | `const unordered_set<string>&` | All tags |
| `IsActive()` / `SetActive(bool)` | `bool` / `void` | Active state |

**Transform:**
| Method | Returns | Description |
|---|---|---|
| `GetPosition()` / `SetPosition(vec3)` | `vec3` / `void` | Local position |
| `GetRotation()` / `SetRotation(quat)` | `quat` / `void` | Local rotation |
| `GetScale()` / `SetScale(vec3)` | `vec3` / `void` | Local scale |
| `SetEulerAngles(vec3)` | `void` | Set rotation from euler degrees |
| `GetEulerAngles()` | `vec3` | Get rotation as euler degrees |
| `GetTransformMatrix()` | `mat4` | Local TRS matrix (cached) |
| `Forward()` / `Right()` / `Up()` | `vec3` | Local axes |
| `Translate(delta)` | `void` | Relative translation |
| `Rotate(eulerDeg)` | `void` | Relative rotation |
| `RotateAround(point, axis, angleDeg)` | `void` | Rotate around a point |
| `LookAt(target)` | `void` | Face a world position |
| `GetWorldPosition()` | `vec3` | World-space position (respects hierarchy) |
| `GetWorldRotation()` | `quat` | World-space rotation |
| `GetWorldScale()` | `vec3` | World-space scale |
| `SetWorldPosition/Rotation/Scale` | `void` | Set in world space |

**Hierarchy:**
| Method | Returns | Description |
|---|---|---|
| `SetParent(Entity*)` | `void` | Attach to parent |
| `GetParent()` | `Entity*` | Parent |
| `GetChildren()` | `const vector<Entity*>&` | Direct children |
| `IsRoot()` | `bool` | No parent? |
| `HasParent()` | `bool` | Has parent? |
| `IsAncestorOf(other)` | `bool` | Ancestor check |
| `GetChildrenRecursive()` | `vector<Entity*>` | All descendants |
| `DestroyChildren()` | `void` | Remove all children |
| `DetachFromParent()` | `void` | Unparent |

**Component Management (templated):**
| Method | Returns | Description |
|---|---|---|
| `AddComponent<T>(args...)` | `T*` | Add component |
| `GetComponent<T>()` | `T*` | Get component by type |
| `HasComponent<T>()` | `bool` | Check type exists |
| `RemoveComponent<T>()` | `void` | Remove by type |
| `GetComponentTypes()` | `vector<type_index>` | All registered component types |
| `GetComponentsInChildren<T>()` | `vector<T*>` | Search recursively |

**Utility:**
| Method | Returns | Description |
|---|---|---|
| `DistanceTo(other)` | `float` | Distance to another entity |
| `DistanceTo(point)` | `float` | Distance to world point |

---

### Component

Base class `planet::Component`.

| Method | Returns | Description |
|---|---|---|
| `GetOwner()` | `Entity*` | Owning entity |
| `OnInit()` | `void` | Virtual, called on creation |
| `OnUpdate(dt)` | `void` | Virtual, called every frame |
| `OnRender()` | `void` | Virtual, called for rendering |
| `OnDestroy()` | `void` | Virtual, called on removal |
| `IsActive()` / `SetActive(bool)` | `bool` / `void` | Active state |

---

### Built-in Components

All in `planet` namespace, inherit `Component`.

**TransformComponent:**
| Field | Type | Default |
|---|---|---|
| `position` | `glm::vec3` | `(0,0,0)` |
| `rotation` | `glm::quat` | Identity |
| `scale` | `glm::vec3` | `(1,1,1)` |
| `Forward()` / `Right()` / `Up()` | `vec3` | Local axes |
| `GetMatrix()` | `mat4` | TRS matrix |

**MeshComponent:**
| Field | Type | Default |
|---|---|---|
| `mesh` | `Mesh*` | `nullptr` |
| `texture` | `Texture*` | `nullptr` |
| `color` | `glm::vec4` | `(1,1,1,1)` |
| `wireframe` | `bool` | `false` |
| `visible` | `bool` | `true` |
| `unlit` | `bool` | `false` |

**SpriteComponent:**
| Field | Type | Default |
|---|---|---|
| `texture` | `Texture*` | `nullptr` |
| `size` | `glm::vec2` | `(1,1)` |
| `color` | `glm::vec4` | `(1,1,1,1)` |
| `rotation` | `float` | `0` |
| `layer` | `int` | `0` |
| `visible` | `bool` | `true` |

**RigidbodyComponent:**
| Field | Type | Default |
|---|---|---|
| `mass` | `float` | `1.0` |
| `velocity` | `glm::vec3` | `(0,0,0)` |
| `angularVelocity` | `glm::vec3` | `(0,0,0)` |
| `useGravity` | `bool` | `true` |
| `isKinematic` | `bool` | `false` |
| `drag` | `float` | `0.0` |
| `angularDrag` | `float` | `0.05` |

**ColliderComponent:**
| Field | Type | Default |
|---|---|---|
| `type` | `enum { Box, Sphere, Capsule, Mesh }` | `Box` |
| `size` | `glm::vec3` | `(1,1,1)` |
| `center` | `glm::vec3` | `(0,0,0)` |
| `isTrigger` | `bool` | `false` |

**ScriptComponent:**
| Field | Type | Description |
|---|---|---|
| `scriptPath` | `string` | Path to Lua script |

**LightComponent:**
| Field | Type | Default |
|---|---|---|
| `type` | `enum { Directional, Point, Spot }` | `Point` |
| `color` | `glm::vec3` | `(1,1,1)` |
| `intensity` | `float` | `1.0` |
| `range` | `float` | `10.0` |
| `spotAngle` | `float` | `30.0` |

**CameraComponent:**
| Field | Type | Default |
|---|---|---|
| `fov` | `float` | `60.0` |
| `nearPlane` | `float` | `0.1` |
| `farPlane` | `float` | `1000.0` |
| `isOrthographic` | `bool` | `false` |
| `orthoSize` | `float` | `10.0` |
| `isMain` | `bool` | `false` |

**AudioSourceComponent:**
| Field | Type | Default |
|---|---|---|
| `soundPath` | `string` | — |
| `volume` | `float` | `1.0` |
| `pitch` | `float` | `1.0` |
| `loop` | `bool` | `false` |
| `playOnStart` | `bool` | `true` |
| `minDistance` | `float` | `1.0` |
| `maxDistance` | `float` | `50.0` |
| `is3D` | `bool` | `true` |
| `sourceId` | `int` | `-1` |

**ParticleEmitterComponent:**
| Field | Type | Default |
|---|---|---|
| `maxParticles` | `int` | `100` |
| `emissionRate` | `float` | `10.0` |
| `lifetime` | `float` | `1.0` |
| `speed` | `float` | `5.0` |
| `startColor` | `glm::vec4` | `(1,1,1,1)` |
| `endColor` | `glm::vec4` | `(0,0,0,0)` |
| `startSize / endSize` | `float` | `0.1` / `0.0` |
| `gravityModifier` | `glm::vec3` | `(0,-0.5,0)` |
| `looping` | `bool` | `true` |
| `playing` | `bool` | `false` |

**AnimationComponent:**
| Field | Type | Description |
|---|---|---|
| `animationName` | `string` | Animation identifier |
| `time` | `float` | Current playback time |
| `speed` | `float` | Playback speed |
| `looping` | `bool` | Loop flag |
| `playing` | `bool` | Play state |
| `keyframes` | `vector<Keyframe>` | Animation keyframes |
| *Keyframe:* `time`, `position`, `rotation`, `scale` | — | Per-keyframe data |

**BillboardComponent:**
| Field | Type | Default |
|---|---|---|
| `texture` | `Texture*` | `nullptr` |
| `size` | `glm::vec2` | `(1,1)` |
| `color` | `glm::vec4` | `(1,1,1,1)` |
| `lockY` | `bool` | `true` (only rotates around Y) |

**TriggerComponent:**
| Field | Type | Default |
|---|---|---|
| `size` | `glm::vec3` | `(1,1,1)` |
| `oneShot` | `bool` | `false` |
| `onEnterScript` | `string` | Script to run on enter |
| `onExitScript` | `string` | Script to run on exit |
| `fired` | `bool` | `false` |
| `overlapping` | `unordered_set<Entity*>` | Current overlaps |

---

### Systems

**System** base class: `virtual void Update(double dt) = 0`.

**Built-in System Implementations:**
| Class | Description |
|---|---|
| `MeshRenderSystem` | Submits MeshComponents to ModelRenderer |
| `SpriteRenderSystem` | Submits SpriteComponents to SpriteRenderer |
| `PhysicsSystem` | Integrates RigidbodyComponents |
| `ScriptSystem` | Runs ScriptComponent Lua scripts |

**SystemManager (singleton):**
| Method | Returns | Description |
|---|---|---|
| `Instance()` | `SystemManager&` | Singleton |
| `RegisterSystem(unique_ptr<System>)` | `void` | Add system |
| `UpdateAll(double dt)` | `void` | Call Update on all |
| `GetSystem<T>()` | `T*` | Get by type |

---

## 4. Physics

### Physics

Singleton class `planet::Physics`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Physics&` | Singleton |
| `Init()` / `Shutdown()` | `void` | Lifecycle |
| `Update(float dt)` | `void` | Physics step (with sub-steps) |
| `SetGravity(vec3)` | `void` | Gravity vector |
| `GetGravity()` | `vec3` | Current gravity |
| `Integrate(RigidbodyComponent*, Entity*, dt)` | `void` | Euler integration |
| `Raycast(origin, dir, maxDist, hitInfo)` | `bool` | Sphere-based raycast |
| `RaycastAll(origin, dir, maxDist, hits)` | `bool` | All hits sorted by distance |
| `OverlapSphere(center, radius)` | `vector<OverlapResult>` | Entities in sphere |
| `OverlapBox(center, halfSize)` | `vector<OverlapResult>` | Entities in box |
| `CheckCollision(a, b)` | `bool` | Sphere-sphere check |
| `GetAllCollisions()` | `vector<pair>` | All pairs in collision |
| `SetSubSteps(int)` | `void` | Sub-step count (default 1) |
| `GetSubSteps()` | `int` | Current sub-steps |

**RaycastHit struct:**
| Field | Type | Description |
|---|---|---|
| `entity` | `Entity*` | Hit entity |
| `point` | `glm::vec3` | Hit world position |
| `normal` | `glm::vec3` | Surface normal *(not computed)* |
| `distance` | `float` | Distance from origin |

**OverlapResult struct:**
| Field | Type | Description |
|---|---|---|
| `entity` | `Entity*` | Overlapping entity |
| `distance` | `float` | Distance from center |

---

## 5. Audio

### Audio (OpenAL)

Singleton class `planet::Audio`. Available when `PLANET_USE_FMOD` is OFF and `PLANET_NO_AUDIO` is OFF.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `Audio&` | Singleton |
| `Init()` / `Shutdown()` / `Update()` | `bool` / `void` | Lifecycle |
| `PlaySound(path, volume?, loop?)` | `int` | Play SFX, returns source ID |
| `StopSound(int id)` | `void` | Stop and free |
| `IsSoundPlaying(int id)` | `bool` | Playing check |
| `SetSoundVolume(id, vol)` | `void` | Per-source volume |
| `SetSoundPitch(id, pitch)` | `void` | Per-source pitch |
| `StopAllSounds()` | `void` | Stop and clear all |
| `PauseSound(id)` / `ResumeSound(id)` | `void` | Pause/resume single |
| `PauseAllSounds()` / `ResumeAllSounds()` | `void` | Pause/resume all |
| `GetSoundDuration(id)` | `float` | Duration in seconds |
| `GetSoundPosition(id)` / `SetSoundPosition(id, pos)` | `float` / `void` | Seek (seconds) |
| `PlayMusic(path, volume?)` | `void` | Looping music |
| `StopMusic()` | `void` | Stop music |
| `IsMusicPlaying()` | `bool` | Music playing? |
| `SetMasterVolume(float)` | `void` | Master gain |
| `GetMasterVolume()` | `float` | Current master |
| `SetDopplerFactor(float)` | `void` | Doppler effect |
| `SetSpeedOfSound(float)` | `void` | Speed of sound |

Format support: WAV only (via built-in loader). Uses `AL_INVERSE_DISTANCE_CLAMPED` distance model.

---

### FmodAudio

Singleton class `planet::FmodAudio`. Identical API to `Audio` above, selected at compile time via `PLANET_USE_FMOD=ON`.

Same methods: `PlaySound`, `StopSound`, `IsSoundPlaying`, `SetSoundVolume`, `SetSoundPitch`, `StopAllSounds`, `PauseSound`, `ResumeSound`, `PauseAllSounds`, `ResumeAllSounds`, `GetSoundDuration`, `GetSoundPosition`, `SetSoundPosition`, `PlayMusic`, `StopMusic`, `IsMusicPlaying`, `SetMasterVolume`, `GetMasterVolume`.

Uses FMOD `System` with 64 channels.

---

### Audio Stub

When `PLANET_NO_AUDIO=ON` (default for Windows cross-compile), all audio methods are no-ops. `PlaySound` returns -1.

---

## 6. Debug

### DebugServer

Singleton class `planet::DebugServer`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `DebugServer&` | Singleton |
| `Start(port?)` | `void` | Start server thread (default port 9876) |
| `Stop()` | `void` | Stop server |
| `IsRunning()` | `bool` | Server state |
| `Update()` | `void` | Send pending events |
| `NotifyScriptInvoke(entity, scriptPath)` | `void` | Log script invocation |

Sends JSON snapshots of entity state to connected TCP clients. Each snapshot includes entity name, position, rotation, scale, tags, and component list.

---

## 7. Resource

### ResourceManager

Singleton class `planet::ResourceManager`.

| Method | Returns | Description |
|---|---|---|
| `Instance()` | `ResourceManager&` | Singleton |
| `Init(assetPath)` / `Shutdown()` | `void` | Lifecycle |
| `LoadKerdata(path)` | `void` | Load kerdata archive |
| `HasKerdata()` | `bool` | Kerdata loaded? |
| `ReadKerdataFile(path, outData)` | `bool` | Read from archive |
| `LoadTexture(path)` | `Texture*` | Load or retrieve cached |
| `LoadMesh(path)` | `Mesh*` | *(not implemented)* |
| `LoadShader(name, vertPath, fragPath)` | `Shader*` | Load or retrieve |
| `GetTexture/Mesh/Shader` | — | Cached lookup |
| `GetAssetPath()` | `string` | Asset root |
| `GetFullPath(relative)` | `string` | Resolve to absolute |
| `Exists(path)` | `bool` | Check file existence |
| `UnloadTexture/Mesh/Shader` | `void` | Remove from cache |
| `UnloadAll()` | `void` | Clear all caches |
| `GetLoadedTexturePaths()` | `vector<string>` | Cached texture paths |
| `GetLoadedMeshPaths()` | `vector<string>` | Cached mesh paths |
| `GetLoadedShaderNames()` | `vector<string>` | Cached shader names |
| `GetDefaultShader()` | `Shader*` | Built-in default |
| `FileExists(path)` | `bool` | Static file check |
| `ReadFile(path)` | `string` | Read file contents |

Resource caching is by path/name. Textures are loaded with stb_image (flipped vertically, auto-mipmap for POT).

---

## 8. Lua Bindings

All engine features are exposed to Lua via global tables. Lists below show each function with its Lua signature.

### Lua Engine Table

`Engine.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `log(msg)` | `string` | — | Print to terminal |
| `logWarning(msg)` | `string` | — | Console warning |
| `logError(msg)` | `string` | — | Console error |
| `quit()` | — | — | Request engine exit |
| `getDeltaTime()` | — | `number` | Frame delta time |
| `getElapsedTime()` | — | `number` | Total elapsed time |
| `getFrameCount()` | — | `int` | Frames rendered |
| `getAverageFPS()` | — | `number` | Average FPS |
| `getTimeSinceStart()` | — | `number` | Wall clock time |
| `setTargetFPS(fps)` | `int` | — | Set FPS cap |
| `getTargetFPS()` | — | `int` | Get FPS cap |
| `setVSync(bool)` | `bool` | — | Toggle vsync |
| `isVSync()` | — | `bool` | Vsync state |
| `setFullscreen(bool)` | `bool` | — | Toggle fullscreen |
| `isFullscreen()` | — | `bool` | Fullscreen state |
| `setWindowTitle(title)` | `string` | — | Set window title |
| `setWindowIcon(path)` | `string` | — | Set window icon |
| `getWindowSize()` | — | `int, int` | Width, height |
| `setWindowSize(w, h)` | `int, int` | — | Resize window |
| `setWindowFullscreen(bool)` | `bool` | — | Toggle (alias) |
| `minimizeWindow()` | — | — | Iconify |
| `restoreWindow()` | — | — | Restore |
| `getNoclip()` | — | `bool` | Noclip state |
| `getNoclipVerticalDelta(dt, speed)` | `number, number` | `number` | Noclip vertical input |

### Lua Camera Table

`Camera.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `setPosition(x,y,z)` | `number,number,number` | — | Camera position |
| `getPosition()` | — | `number,number,number` | Camera position |
| `setTarget(x,y,z)` | `number,number,number` | — | Look-at target |
| `setFOV(fov)` | `number` | — | Field of view |
| `getFOV()` | — | `number` | Current FOV |
| `orbit(yawDeg, pitchDeg)` | `number,number` | — | Orbit camera |
| `lookAt(x,y,z)` | `number,number,number` | — | Point at position |
| `setZoom(zoom)` | `number` | — | Zoom factor |
| `getZoom()` | — | `number` | Current zoom |
| `setPitch(deg)` | `number` | — | Pitch angle |
| `setYaw(deg)` | `number` | — | Yaw angle |
| `setMouseLock(bool)` | `bool` | — | Lock cursor |
| `screenToWorld(sx,sy,sz?,vw?,vh?)` | `number...` | `number,number,number` | Unproject |
| `worldToScreen(wx,wy,wz,vw?,vh?)` | `number...` | `number,number` | Project |

### Lua Scene Table

`Scene.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `clear()` | — | — | Remove all entities |
| `getEntityCount()` | — | `int` | Number of entities |
| `findEntities(name)` | `string` | `{table,...}` | Find by name |
| `setAmbientLight(r,g,b)` | `number,number,number` | — | Ambient color |

### Lua Console Table

`Console.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `addCommand(name,prefix,args,desc,func)` | `string,string,string,string,function` | — | Register command |
| `print(msg)` | `string` | — | Print to console |

### Lua Renderer Table

`Renderer.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `clearColor(r,g,b,a?)` | `number,number,number,number?` | — | Set clear color |
| `getClearColor()` | — | `4x number` | Clear color |
| `setWireframe(bool)` | `bool` | — | Wireframe toggle |
| `isWireframe()` | — | `bool` | Wireframe state |
| `drawSprite(x,y,w,h,r?,g?,b?,a?)` | `number...` | — | Colored quad |
| `drawRect(x,y,w,h,r?,g?,b?,a?)` | `number...` | — | Alias for drawSprite |
| `setLightDir(x,y,z)` | `number,number,number` | — | Directional light |
| `setLightColor(r,g,b)` | `number,number,number` | — | Light color |
| `setAmbientColor(r,g,b)` | `number,number,number` | — | Ambient |
| `getLightDir()` | — | `3x number` | Light direction |
| `getAmbientColor()` | — | `3x number` | Ambient color |
| `setFogColor(r,g,b)` | `number,number,number` | — | Fog tint |
| `setFogDensity(d)` | `number` | — | Exponential density |
| `setFogRange(start,end)` | `number,number` | — | Linear range |
| `setFogEnabled(bool)` | `bool` | — | Fog toggle |
| `isFogEnabled()` | — | `bool` | Fog state |
| `setPostEffect(name)` | `string` | — | Effect: none/vhs/dream/vignette/... |
| `getPostEffect()` | — | `string` | Current effect name |
| `setPostIntensity(intensity)` | `number` | — | Effect intensity |
| `getPostIntensity()` | — | `number` | Intensity |
| `setFlashlight(enabled, ...)` | `bool, 9x number?` | — | Flashlight control |
| `drawText(text,x,y,scale?,r?,g?,b?,a?)` | `string, number...` | — | Text rendering |
| `drawTextCentered(text,cx,y,scale?,r?,g?,b?,a?)` | `string, number...` | — | Centered text |
| `addPointLight(x,y,z,r,g,b,range?)` | `number...` | `int` | Add light |
| `removePointLight(index)` | `int` | — | Remove |
| `clearPointLights()` | — | — | Clear all |
| `setPointLightPosition(idx,x,y,z)` | `int, number...` | — | Set pos |
| `setPointLightColor(idx,r,g,b)` | `int, number...` | — | Set color |
| `getPointLightCount()` | — | `int` | Light count |
| `loadTexture(path)` | `string` | `table or nil` | Load texture |

### Lua Input Table

`Input.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `getKey(name)` | `string` | `bool` | Key held (name: "w", "space", "escape", etc.) |
| `getKeyDown(name)` | `string` | `bool` | Key just pressed |
| `getKeyUp(name)` | `string` | `bool` | Key just released |
| `getMousePosition()` | — | `number, number` | Cursor X, Y |
| `getMouseDelta()` | — | `number, number` | Mouse movement |
| `getScrollDelta()` | — | `number` | Scroll wheel |
| `getMouseButton(button)` | `int` (0=left,1=right,2=middle) | `bool` | Button held |
| `getMouseButtonDown(button)` | `int` | `bool` | Button pressed |
| `getMouseButtonUp(button)` | `int` | `bool` | Button released |
| `setMouseLocked(bool)` | `bool` | — | Lock cursor |
| `isMouseLocked()` | — | `bool` | Mouse lock state |
| `getAnyKeyDown()` | — | `bool` | Any key pressed? |
| `getAnyKey()` | — | `bool` | Any key held? |

**Key name strings:** a-z, 0-9, space, escape, enter, tab, backspace, delete, insert, up/down/left/right, pageup, pagedown, home, end, left_shift, right_shift, left_ctrl, right_ctrl, left_alt, right_alt, left_super, right_super, f1-f12.

### Lua ECS Table

`ECS.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `createEntity(name?)` | `string?` | `table` | Create entity |
| `findEntity(name)` | `string` | `table or nil` | Find by name |
| `destroyEntity(entity)` | `table` | — | Destroy |
| `entitySetPosition(e,x,y,z)` | `table, 3x number` | — | Set position |
| `entityGetPosition(e)` | `table` | `3x number` | Get position |
| `entitySetScale(e,x,y,z)` | `table, 3x number` | — | Set scale |
| `entitySetRotation(e,x,y,z)` | `table, 3x number` | — | Set euler rotation |
| `entityGetRotation(e)` | `table` | `3x number` | Get euler rotation |
| `entityAddTag(e,tag)` | `table, string` | — | Add tag |
| `entityHasTag(e,tag)` | `table, string` | `bool` | Check tag |
| `entityRemoveTag(e,tag)` | `table, string` | — | Remove tag |
| `entitySetActive(e,bool)` | `table, bool` | — | Set active |
| `entityIsActive(e)` | `table` | `bool` | Check active |
| `entityTranslate(e,dx,dy,dz)` | `table, 3x number` | — | Relative move |
| `entityRotate(e,ex,ey,ez)` | `table, 3x number` | — | Relative rotate |
| `entityLookAt(e,x,y,z)` | `table, 3x number` | — | Face point |
| `entityGetForward(e)` | `table` | `3x number` | Forward vector |
| `entityGetRight(e)` | `table` | `3x number` | Right vector |
| `entityGetUp(e)` | `table` | `3x number` | Up vector |
| `entitySetParent(e,parent)` | `table, table` | — | Parent |
| `entityGetParent(e)` | `table` | `table or nil` | Get parent |
| `entityDistanceTo(a,b)` | `table, table` | `number` | Distance |
| `entitySetColor(e,r,g,b,a?)` | `table, 3-4x number` | — | MeshComponent color |
| `entitySetUnlit(e,bool)` | `table, bool` | — | Unlit flag |
| `entityGetName(e)` | `table` | `string` | Name |
| `entityGetScale(e)` | `table` | `3x number` | Scale |
| `entitySetTexture(e,path)` | `table, string` | — | Set MeshComponent texture |
| `entityAddScript(e,path)` | `table, string` | — | Add ScriptComponent |
| `entityAddRigidbody(e,mass?)` | `table, number?` | — | Add RigidbodyComponent |
| `entityAddLight(e,type?,r?,g?,b?,range?)` | `table, string?, 4x number?` | — | Add LightComponent |
| `addMeshBox(e,size?,r?,g?,b?,a?,wire?)` | `table, number...` | — | Add cube MeshComponent |
| `addMeshPlane(e,w?,d?,r?,g?,b?,a?)` | `table, number...` | — | Add plane |
| `addMeshBoxEx(e,w,h,d)` | `table, 3x number` | — | Add box |
| `addMeshWallQuad(e,w,h)` | `table, 2x number` | — | Add wall quad |
| `addMeshSphere(e,radius?,segs?,r?,g?,b?,a?,wire?)` | `table, number...` | — | Add sphere |
| `addMeshCylinder(e,radius?,height?,segs?,r?,g?,b?,a?)` | `table, number...` | — | Add cylinder |
| `addMeshCone(e,radius?,height?,segs?,r?,g?,b?,a?)` | `table, number...` | — | Add cone |
| `addMeshTorus(e,radius?,tube?,segs?,tubeSegs?,r?,g?,b?,a?)` | `table, number...` | — | Add torus |

### Lua Physics Table

`Physics.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `setGravity(x,y,z?)` | `number,number,number?` | — | Set gravity |
| `getGravity()` | — | `3x number` | Get gravity |
| `raycast(ox,oy,oz,dx,dy,dz,maxDist?)` | `7x number` | `bool or (true, name, hx, hy, hz, dist, ptr)` | Raycast |
| `overlapSphere(cx,cy,cz,radius)` | `4x number` | `{table,...}` | Overlap test |
| `setSubSteps(steps)` | `int` | — | Sub-step count |

### Lua Audio Table

`Audio.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `playSound(path, volume?, loop?)` | `string, number?, bool?` | `int` | Play sound |
| `stopSound(id)` | `int` | — | Stop |
| `isSoundPlaying(id)` | `int` | `bool` | Playing? |
| `setSoundVolume(id, vol)` | `int, number` | — | Volume |
| `setSoundPitch(id, pitch)` | `int, number` | — | Pitch |
| `pauseSound(id)` | `int` | — | Pause |
| `resumeSound(id)` | `int` | — | Resume |
| `stopAllSounds()` | — | — | Stop all |
| `pauseAllSounds()` | — | — | Pause all |
| `resumeAllSounds()` | — | — | Resume all |
| `playMusic(path, volume?)` | `string, number?` | — | Music |
| `stopMusic()` | — | — | Stop music |
| `isMusicPlaying()` | — | `bool` | Music check |
| `setMasterVolume(vol)` | `number` | — | Master |
| `getMasterVolume()` | — | `number` | Master |

### Lua Portal Table

`Portal.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `create(name,px,py,pz,rx?,ry?,rz?,w?,h?)` | `string, 3x number, 3x number?, 2x number?` | `int` | Create portal |
| `link(a,b)` | `int, int` | — | Link portals |
| `setColor(id,r,g,b,a?)` | `int, 3-4x number` | — | Portal color |
| `setActive(id,bool)` | `int, bool` | — | Enable/disable |
| `setPosition(id,x,y,z)` | `int, 3x number` | — | Move portal |
| `destroy(id)` | `int` | — | Remove portal |
| `clearAll()` | — | — | Remove all |
| `checkTeleport(ox,oy,oz,nx,ny,nz)` | `6x number` | `bool or (true, wx,wy,wz, fx,fy,fz)` | Teleport |
| `getPosition(id)` | `int` | `3x number or nil` | Position |
| `getForward(id)` | `int` | `3x number or nil` | Forward |
| `getSize(id)` | `int` | `2x number or nil` | Size |
| `getCount()` | — | `int` | Portal count |
| `setRecursionDepth(depth)` | `int` | — | Recursion [0-4] |

### Lua Dexecl Table

`Dexecl.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `invoke(filename)` | `string` | `bool` | Execute .dexl file |
| `consoleInvoke(cmd)` | `string` | — | Execute console command |
| `setVar(name, value)` | `string, number` | — | Set variable |
| `getVar(name, default?)` | `string, number?` | `number` | Get variable |

**Global aliases:** `dexeclInvoke(filename)` and `consoleInvoke(cmd)`.

### Lua Math Table

`Math.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `lerp(a, b, t)` | `3x number` | `number` | Linear interpolation |
| `clamp(v, min, max)` | `3x number` | `number` | Clamp value |
| `smoothStep(e0, e1, x)` | `3x number` | `number` | Smooth step |
| `map(v, iMin, iMax, oMin, oMax)` | `5x number` | `number` | Remap range |
| `pingPong(t, length)` | `2x number` | `number` | Ping-pong |
| `moveTowards(cur, target, maxDelta)` | `3x number` | `number` | Move towards |
| `deltaAngle(cur, target)` | `2x number` | `number` | Angle difference |

### Lua Random Table

`Random.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `range(min?, max?)` | `2x number?` | `number` | Random float [min, max) |
| `int(min, max)` | `2x int` | `int` | Random integer |
| `unitVector()` | — | `3x number` | Random unit vec3 |
| `insideUnitSphere()` | — | `3x number` | Random point in sphere |
| `setSeed(seed)` | `int` | — | Seed RNG |

### Lua Timer Table

`Timer.*`

| Function | Args | Returns | Description |
|---|---|---|---|
| `add(interval, func, looping?)` | `number, function, bool?` | `int` | Create timer |
| `remove(id)` | `int` | — | Remove timer |
| `delay(seconds, func)` | `number, function` | `int` | Delayed call |

---

## 9. Platform Support Notes

| Feature | Linux | Windows (MinGW) | macOS |
|---|---|---|---|
| Window | GLFW | GLFW | GLFW |
| OpenGL | 4.1+ Core | 4.1+ Core | 4.1+ Core |
| Audio (OpenAL) | ✅ Full | ❌ Not supported | ✅ Full |
| Audio (FMOD) | ✅ Optional | ✅ Optional | ✅ Optional |
| No-Audio Stub | ✅ Available | ✅ Default | ✅ Available |
| Input | ✅ Full | ✅ Full | ✅ Full |
| Portals | ✅ | ✅ | ✅ |
| Post-Processing | ✅ (3 effects) | ✅ (3 effects) | ✅ (3 effects) |
| Kerdata Archives | ✅ | ✅ | ✅ |
| Debug Server | ✅ (TCP) | ✅ (TCP) | ✅ (TCP) |
| Dexecl Scripting | ✅ | ✅ | ✅ |
| Lua Scripting | ✅ 5.4 | ✅ 5.4 | ✅ 5.4 |
| Console (In-Game) | ✅ | ✅ | ✅ |
| Threading | ✅ pthread | ✅ Win32 | ✅ pthread |
| GLM / GLAD | ✅ | ✅ | ✅ |
| stb_image | ✅ | ✅ | ✅ |

**Build Options:**

| CMake Option | Values | Default | Description |
|---|---|---|---|
| `PLANET_USE_FMOD` | `ON`/`OFF` | `OFF` | Use FMOD audio |
| `PLANET_NO_AUDIO` | `ON`/`OFF` | `OFF` (`ON` on Windows) | Disable audio |
| `PLANET_BUILD_WINDOWS` | `ON`/`OFF` | `OFF` | MinGW cross-compile |
| `ENGINE_BUILD_EXAMPLES` | `ON`/`OFF` | `ON` | Build example projects |
| `BUILD_SHARED_LIBS` | `ON`/`OFF` | `OFF` | Shared libraries |

---

*Generated for Planet Engine v0.2.0 — API reference covers all C++ headers and Lua bindings.*
