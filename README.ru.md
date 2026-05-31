# Planet Engine

Минимальный игровой движок с 3D/2D рендерингом на OpenGL, ECS, физикой и аудио. Поддерживает скриптинг на Lua. Собирается под Linux, Windows и macOS.

## Возможности

- **OpenGL 4.1 Core** рендерер с шейдерами, текстурами, освещением и пост-эффектами (VHS, bloom, CRT и др.)
- **Lua 5.4 скриптинг** для игровой логики, сцен и поведения сущностей
- **Entity Component System** с 8 компонентами (трансформация, меш, камера, свет, физика, скрипт и др.)
- **Система карт** — текстовые `.map` файлы с поддержкой шаблонов, освещения и мешей
- **Консоль** — внутриигровая консоль разработчика (клавиша `ё/backtick`) с 30+ командами
- **Физика** — базовая 3D физика с гравитацией, коллизиями и рейкастингом
- **Аудио** — OpenAL (3D позиционный звук)
- **DEXECL** — декларативный язык конфигурации сущностей
- **Пост-эффекты** — 14 эффектов (виньетка, блюм, ЭЛТ, рыбный глаз и др.)

## Что работает

- ✅ 3D рендеринг с мешами, текстурами, освещением (направленный, точечный, фонарик)
- ✅ Lua скриптинг с полным биндингом API движка
- ✅ ECS с 8 типами компонентов
- ✅ Загрузка карт (текстовый формат и наследуемый Lua)
- ✅ Консоль с 30+ командами
- ✅ Воспроизведение аудио (OpenAL на Linux/macOS/Windows)
- ✅ Физическое моделирование
- ✅ Пост-эффекты
- ✅ Игра-лабиринт (WASD движение, генерация случайного лабиринта, AI врагов)
- ✅ Кросс-компиляция: Linux (native), Windows (MinGW), macOS (ARM64)
- ✅ Рендеринг шрифтов с загрузкой TTF и встроенным растровым шрифтом
- ✅ Рендеринг спрайтов и текста
- ✅ DEXECL конфигурационные файлы

## Что не работает / не завершено

- ❌ Редактор (ncurses TUI) — заархивирован, не собирается
- ❌ Редактор карт (`planet_mapper`) — компилируется, но TUI нерабочий
- ❌ Kerdata архиватор — экспериментальный, не интегрирован
- ❌ LSP клиент — не завершён
- ❌ FMOD аудио — опционально, требуется FMOD SDK
- ❌ Кросс-компиляция в macOS из Linux — требуется osxcross + Xcode SDK
- ❌ Сеть/мультиплеер — не реализовано

## Быстрый старт

### Зависимости

- CMake ≥ 3.20
- Видеокарта с OpenGL 4.1
- GLFW 3.x
- OpenAL (Linux: `libopenal-dev`, macOS: встроенный фреймворк)
- Lua 5.4 (включён в `thirdparty/`)
- pthreads

### Сборка

```bash
cmake -B build
make -C build -j$(nproc)
```

### Запуск игры-лабиринта

```bash
cp -r projects/example/* build/bin/
cd build/bin && ./planet
```

Управление: WASD — движение, Мышь — осмотр, Tab — захват мыши, `ё/Backtick` — консоль, Escape — выход.

### Нативная сборка под Windows

Требуется **Visual Studio 2022** (или 2019) с нагрузкой "Разработка классических приложений на C++", или **MinGW-w64** (UCRT64/MSYS2).

Установка зависимостей через vcpkg:

```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && .\bootstrap-vcpkg.bat
.\vcpkg install glfw3 openal-soft
cd ..\planet
cmake -B build -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build
```

Или установите GLFW и OpenAL вручную через официальные установщики, затем:

```powershell
cmake -B build
cmake --build build
```

Игра и ассеты — в `build\bin\`.

### Кросс-компиляция под Windows (из Linux)

```bash
cmake -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake -DPLANET_BUILD_WINDOWS=ON
make -C build-mingw package_windows
```

### Сборка под macOS

На Apple Silicon Mac:

```bash
cmake -B build
make -C build -j$(sysctl -n hw.ncpu) planet
```

## Структура проекта

```
├── CMakeLists.txt          # Система сборки
├── cmake/                  # Toolchain файлы CMake
├── fonts/                  # Встроенные шрифты
├── projects/               # Игровые проекты
│   └── example/            # Пример игры-лабиринта
├── scripts/                # Вспомогательные скрипты
├── src/
│   ├── main.cpp            # Точка входа
│   ├── planet/             # Ядро движка
│   │   ├── audio/          # OpenAL аудио
│   │   ├── core/           # Окно, ввод, сцена, консоль, камера
│   │   ├── ecs/            # Entity Component System
│   │   ├── editor/         # [Архив] Исходники редактора
│   │   ├── map/            # Загрузчик и парсер карт
│   │   ├── mapper/         # [Архив] TUI редактор карт
│   │   ├── physics/        # Физический движок
│   │   ├── render/         # Рендерер, шейдеры, текст, спрайты
│   │   └── resource/       # Менеджер ресурсов
│   ├── lua/                # Lua движок и биндинги API
│   └── bundler/            # Инструмент архивации Kerdata
└── thirdparty/             # Сторонние библиотеки (Lua, GLAD, stb, glm)
```

## Документация

См. [docs/ENGINE.ru.md](docs/ENGINE.ru.md) — полное описание API движка, формата карт и консольных команд.

## Лицензия

Проект предоставляется «как есть». Сторонние библиотеки (Lua, GLFW, GLAD, stb, glm) распространяются под своими лицензиями — см. [CREDITS.md](CREDITS.md).
