# Документация Planet Engine

## Архитектура

Движок построен вокруг синглтонов и основного цикла:

```
main.cpp → Engine::Run()
  ├── Window (GLFW)
  ├── Input
  ├── Renderer → SpriteRenderer, ModelRenderer, TextRenderer
  ├── PostProcessor (FBO + пост-эффекты)
  ├── Audio (OpenAL или FMOD)
  ├── Physics
  ├── Scene (дерево сущностей)
  ├── LuaRuntime (игровые скрипты)
  ├── Console (внутриигровая консоль)
  ├── SystemManager (ECS системы)
  └── ResourceManager
```

Основной цикл: `PollEvents → Lua onUpdate → Physics → Scene → Render → SwapBuffers`.

## Lua API

Движок предоставляет глобальную таблицу Lua со следующими модулями:

### Core

| Функция | Описание |
|---------|----------|
| `Engine.quit()` | Выход из приложения |
| `Engine.setTargetFPS(n)` | Ограничение FPS |
| `Engine.getDeltaTime()` | Время кадра в секундах |
| `Engine.getElapsedTime()` | Время с запуска |
| `Engine.getAverageFPS()` | Измеренный FPS |
| `Engine.setVSync(bool)` | Вертикальная синхронизация |
| `Engine.setFullscreen(bool)` | Полноэкранный режим |
| `Engine.setClearColor(r,g,b,a)` | Цвет фона |

### Input

| Функция | Описание |
|---------|----------|
| `Input.getKey(key)` | Клавиша зажата (строка) |
| `Input.getKeyDown(key)` | Клавиша нажата в этом кадре |
| `Input.getKeyUp(key)` | Клавиша отпущена |
| `Input.getMouseX()` / `getMouseY()` | Позиция мыши |
| `Input.getMouseDX()` / `getMouseDY()` | Дельта мыши |
| `Input.getMouseButton(n)` | Состояние кнопки мыши |
| `Input.setMouseLock(bool)` | Захват курсора |
| `Input.isMouseLocked()` | Курсор захвачен? |
| `Input.getScrollX()` / `getScrollY()` | Скролл колеса |

Названия клавиш: `"w"`, `"a"`, `"s"`, `"d"`, `"space"`, `"escape"`, `"tab"`, `"grave_accent"`, `"enter"`, `"shift"` и т.д.

### ECS

| Функция | Описание |
|---------|----------|
| `ECS.createEntity(name)` | Создать сущность, возвращает ID |
| `ECS.removeEntity(id)` | Удалить сущность |
| `ECS.getEntity(id)` | Получить сущность по ID |
| `ECS.findEntity(name)` | Найти по имени |
| `ECS.getEntityCount()` | Всего сущностей |
| `ECS.getEntityName(id)` | Имя сущности |
| `ECS.entityAddComponent(id, type, props)` | Добавить компонент |
| `ECS.entityGetComponent(id, type)` | Получить таблицу компонента |
| `ECS.entityGetPosition(id)` | Возвращает `{x,y,z}` |
| `ECS.entitySetPosition(id, x,y,z)` | Установить позицию |
| `ECS.entityGetRotation(id)` | Возвращает `{x,y,z,w}` кватернион |
| `ECS.entitySetRotation(id, x,y,z,w)` | Установить поворот |
| `ECS.entityGetScale(id)` | Возвращает `{x,y,z}` |
| `ECS.entitySetScale(id, x,y,z)` | Установить масштаб |
| `ECS.entitySetParent(childId, parentId)` | Привязать к родителю |
| `ECS.entitySetTexture(id, path)` | Установить текстуру меша |
| `ECS.entitySetColor(id, r,g,b,a)` | Цвет меша |

Типы компонентов: `"transform"`, `"mesh"`, `"camera"`, `"light"`, `"physics"`, `"script"`, `"portal"`, `"listener"`, `"point_light"`

### Camera

| Функция | Описание |
|---------|----------|
| `Camera.create()` | Создать камеру, возвращает handle |
| `Camera.setActive(handle)` | Сделать активной |
| `Camera.setPosition(h, x,y,z)` | Позиция камеры |
| `Camera.setRotation(h, x,y,z)` | Поворот (Euler) |
| `Camera.setFov(h, fov)` | Поле зрения (градусы) |
| `Camera.setNearPlane(h, n)` | Ближняя плоскость отсечения |
| `Camera.setFarPlane(h, f)` | Дальняя плоскость отсечения |
| `Camera.setType(h, type)` | `"perspective"` или `"orthographic"` |

### Renderer

| Функция | Описание |
|---------|----------|
| `Renderer.setWindowIcon(path)` | Иконка окна |
| `Renderer.getWidth()` | Ширина фреймбуфера (пиксели) |
| `Renderer.getHeight()` | Высота фреймбуфера |
| `Renderer.setLightDirection(x,y,z)` | Направление направленного света |
| `Renderer.setLightColor(r,g,b)` | Цвет направленного света |
| `Renderer.setAmbientColor(r,g,b)` | Фоновое освещение |
| `Renderer.setFogColor(r,g,b)` | Цвет тумана |
| `Renderer.setFogDensity(d)` | Плотность тумана |
| `Renderer.setFogStart(s)` | Начало тумана |
| `Renderer.setFogEnd(e)` | Конец тумана |
| `Renderer.setFogEnabled(bool)` | Включить туман |
| `Renderer.setFlashlight(bool)` | Включить фонарик |
| `Renderer.setFlashlightPos(x,y,z)` | Позиция фонарика |
| `Renderer.setFlashlightDir(x,y,z)` | Направление фонарика |
| `Renderer.setFlashlightColor(r,g,b)` | Цвет фонарика |
| `Renderer.setFlashlightCutoff(c)` | Угол конуса фонарика |
| `Renderer.setRenderMode(bool)` | Wireframe режим |
| `Renderer.addPointLight(x,y,z, r,g,b, range)` | Добавить точечный свет |
| `Renderer.removePointLight(id)` | Удалить точечный свет |

### Sprite

| Функция | Описание |
|---------|----------|
| `Sprite.draw(texture, x, y, w, h)` | Нарисовать спрайт |
| `Sprite.drawRotated(texture, x, y, w, h, angle)` | Повёрнутый спрайт |
| `Sprite.drawColor(texture, x, y, w, h, r,g,b,a)` | Спрайт с цветом |
| `Sprite.drawString(text, x, y, scale, r,g,b,a)` | Текст поверх |
| `Sprite.drawStringCentered(text, cx, y, scale, r,g,b,a)` | Центрированный текст |
| `Sprite.measureWidth(text, scale)` | Ширина текста |

### Audio

| Функция | Описание |
|---------|----------|
| `Audio.loadSound(path)` | Загрузить звук, возвращает handle |
| `Audio.playSound(h)` | Проиграть один раз |
| `Audio.playSoundLoop(h)` | Зациклить |
| `Audio.stopSound(h)` | Остановить |
| `Audio.setVolume(h, vol)` | 0.0 – 1.0 |
| `Audio.setPitch(h, pitch)` | Скорость воспроизведения |
| `Audio.setPosition(h, x,y,z)` | 3D позиция |
| `Audio.isPlaying(h)` | Играет ли сейчас |
| `Audio.setMasterVolume(vol)` | Глобальная громкость |
| `Audio.setListenerPosition(x,y,z)` | Позиция слушателя |
| `Audio.setListenerOrientation(fx,fy,fz, ux,uy,uz)` | Направление слушателя |

### Physics

| Функция | Описание |
|---------|----------|
| `Physics.raycast(ox,oy,oz, dx,dy,dz, maxDist)` | Рейкаст, возвращает `{hit, x,y,z, nx,ny,nz, entityId}` |
| `Physics.setGravity(x,y,z)` | Вектор гравитации |

### Map

| Функция | Описание |
|---------|----------|
| `Map.load(path)` | Загрузить `.map` или `.lua` файл |
| `Map.createObject(templateName, x,y,z)` | Создать из шаблона |
| `Map.setTemplate(name, entityId)` | Зарегистрировать шаблон |

### Console

| Функция | Описание |
|---------|----------|
| `Console.print(...)` | Печать в консоль |
| `Console.clear()` | Очистить консоль |
| `Console.show(bool)` | Показать/скрыть |
| `Console.isOpen()` | Открыта ли |
| `Console.registerCommand(name, fn, help)` | Добавить команду |
| `Console.registerVariable(name, getFn, setFn, help)` | Добавить переменную |

### DEXECL

| Функция | Описание |
|---------|----------|
| `Dexecl.execute(path)` | Выполнить `.dexl` файл |
| `Dexecl.set(key, value)` | Установить переменную |
| `Dexecl.get(key)` | Получить переменную |
| `Dexecl.getEntity(name)` | ID сущности, созданной dexecl |

### Time

| Функция | Описание |
|---------|----------|
| `Time.delay(seconds, callback)` | Одноразовый таймер |
| `Time.interval(seconds, callback)` | Повторяющийся таймер |
| `Time.clear(timerId)` | Отменить таймер |

## Консольные команды

| Команда | Описание |
|---------|----------|
| `help` | Список команд |
| `clear` | Очистить консоль |
| `quit` / `exit` | Выход |
| `echo <text>` | Вывести текст |
| `set <var> <value>` | Установить переменную |
| `get <var>` | Получить переменную |
| `list_vars` | Показать переменные |
| `exec <path>` | Выполнить скрипт |
| `lua <code>` | Выполнить Lua код |
| `load_map <path>` | Загрузить карту |
| `reload` | Перезагрузить карту |
| `show_info` | FPS/сущности |
| `show_light` | Отладка направления света |
| `show_physics` | Отладка физики |
| `screenshot` | Скриншот |
| `fullscreen` | Полный экран |
| `vsync` | Вертикальная синхронизация |
| `fps <n>` | Целевой FPS |
| `wireframe` | Каркасный режим |
| `post_effect <name>` | Пост-эффект |
| `post_intensity <0-1>` | Интенсивность эффекта |
| `fog` | Туман |
| `flashlight` | Фонарик |
| `gravity <x,y,z>` | Гравитация |
| `audio_volume <0-1>` | Громкость |

Пост-эффекты: `none`, `vhs`, `dream`, `vignette`, `grayscale`, `invert`, `blur`, `pixelate`, `edgeDetect`, `sepia`, `bloom`, `crt`, `fishEye`, `motionBlur`, `sharpen`, `emboss`

## Формат карт

Файлы карт используют текстовый формат с расширением `.map`. Каждый объект определяется одной или несколькими строками свойств:

```
[имяОбъекта;флаги]:[ТипМеша](СВОЙСТВО) = значение
```

- **имяОбъекта** — имя для регистрации шаблона; `_none` для объектов только на сцене
- **флаги** — опциональные `ключ=значение` пары через `;`
- **ТипМеша** — форма: `box`, `sphere`, `cylinder`, `plane` или путь к `.obj` файлу
- **СВОЙСТВО** — название свойства
- **значение** — значение свойства

Строки с одинаковым `имяОбъекта` определяют один объект. Именованные объекты становятся шаблонами для `Map.createObject()`.

### Пример

```
[player]:box(POSITION) = 0,1,0
[player]:box(ROTATION) = 0,1,0,0
[player]:box(SCALE) = 0.5,0.5,0.5
[player]:box(COLOR) = 0.2,0.6,1.0,1
[player]:box(TEXTURE) = assets/player.png
[player]:box(COLLISION) = true
[player]:box(SCRIPT) = scripts/player.lua
```

### Свойства

| Свойство | Тип | Описание |
|----------|-----|----------|
| `POSITION` | `x,y,z` | Позиция в мире |
| `ROTATION` | `x,y,z,w` | Поворот (кватернион) |
| `SCALE` | `x,y,z` | Масштаб |
| `COLOR` | `r,g,b,a` | Цвет меша |
| `TEXTURE` | путь | Текстура |
| `SCRIPT` | путь | Lua скрипт |
| `COLLISION` | bool | Физическая коллизия |
| `MASS` | float | Масса |
| `LIGHT_DIRECTIONAL` | bool | Направленный свет |
| `LIGHT_POINT` | bool | Точечный свет |
| `LIGHT_SPOT` | bool | Прожектор |
| `LIGHT_COLOR` | `r,g,b` | Цвет света |
| `LIGHT_RANGE` | float | Дальность точечного света |
| `PORTAL` | string | Целевая карта портала |
| `FLAGS` | string | Флаги |

## Сборка

См. [README.ru.md](../README.ru.md).
