cmake_minimum_required(VERSION 3.16)
project(planet VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENGINE_BUILD_EXAMPLES "Build example projects" ON)
option(PLANET_USE_FMOD "Use FMOD instead of OpenAL for audio" OFF)
option(PLANET_BUILD_WINDOWS "Cross-compiling for Windows/MinGW" OFF)

if(PLANET_BUILD_WINDOWS)
    set(PLANET_NO_AUDIO ON)
    add_compile_definitions(PLANET_BUILD_WINDOWS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
elseif(PLANET_USE_FMOD)
    find_path(FMOD_INCLUDE_DIR fmod.h
        PATHS /usr/local/include/fmod /usr/include/fmod
        DOC "FMOD include directory"
    )
    find_library(FMOD_LIBRARY fmod
        PATHS /usr/local/lib /usr/lib
        DOC "FMOD library"
    )
    if(NOT FMOD_INCLUDE_DIR OR NOT FMOD_LIBRARY)
        message(FATAL_ERROR "PLANET_USE_FMOD=ON but FMOD not found.")
    endif()
    message(STATUS "FMOD found: ${FMOD_INCLUDE_DIR}, ${FMOD_LIBRARY}")
else()
    if(APPLE)
        # On macOS 14+ the OpenAL system framework lacks a Headers symlink;
        # find the SDK headers and link the framework manually.
        find_library(OPENAL_LIBRARY OpenAL)
        if(NOT OPENAL_LIBRARY)
            message(FATAL_ERROR "OpenAL framework not found")
        endif()
        # Try SDK headers first, fall back to framework Headers
        find_path(_OPENAL_HEADERS al.h
            PATHS
                /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenAL.framework/Versions/A/Headers
                /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenAL.framework/Versions/A/Headers
                ${OPENAL_LIBRARY}/Versions/A/Headers
                ${OPENAL_LIBRARY}/Headers
        )
        if(NOT _OPENAL_HEADERS)
            message(FATAL_ERROR "OpenAL headers not found (looked for al.h)")
        endif()
        set(OPENAL_INCLUDE_DIR "${_OPENAL_HEADERS}")
        message(STATUS "OpenAL found: ${OPENAL_LIBRARY} (headers: ${OPENAL_INCLUDE_DIR})")
    else()
        find_package(OpenAL REQUIRED)
    endif()
endif()

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

if(PLANET_BUILD_WINDOWS)
    find_package(glfw3 REQUIRED)
else()
    find_package(glfw3 REQUIRED)
endif()

set(LUA_SOURCES
    thirdparty/lua/src/lapi.c
    thirdparty/lua/src/lauxlib.c
    thirdparty/lua/src/lbaselib.c
    thirdparty/lua/src/lcode.c
    thirdparty/lua/src/lcorolib.c
    thirdparty/lua/src/lctype.c
    thirdparty/lua/src/ldblib.c
    thirdparty/lua/src/ldebug.c
    thirdparty/lua/src/ldo.c
    thirdparty/lua/src/ldump.c
    thirdparty/lua/src/lfunc.c
    thirdparty/lua/src/lgc.c
    thirdparty/lua/src/linit.c
    thirdparty/lua/src/liolib.c
    thirdparty/lua/src/llex.c
    thirdparty/lua/src/lmathlib.c
    thirdparty/lua/src/lmem.c
    thirdparty/lua/src/loadlib.c
    thirdparty/lua/src/lobject.c
    thirdparty/lua/src/lopcodes.c
    thirdparty/lua/src/loslib.c
    thirdparty/lua/src/lparser.c
    thirdparty/lua/src/lstate.c
    thirdparty/lua/src/lstring.c
    thirdparty/lua/src/lstrlib.c
    thirdparty/lua/src/ltable.c
    thirdparty/lua/src/ltablib.c
    thirdparty/lua/src/ltm.c
    thirdparty/lua/src/lundump.c
    thirdparty/lua/src/lutf8lib.c
    thirdparty/lua/src/lvm.c
    thirdparty/lua/src/lzio.c
)

if(PLANET_USE_FMOD)
    set(AUDIO_SOURCES
        src/planet/audio/fmod_audio.cpp
        src/lua/bindings/fmod_audio_bindings.cpp
    )
    set(AUDIO_DEFINES PLANET_USE_FMOD)
elseif(PLANET_NO_AUDIO)
    set(AUDIO_SOURCES
        src/planet/audio/audio_stub.cpp
    )
    set(AUDIO_DEFINES PLANET_NO_AUDIO)
else()
    set(AUDIO_SOURCES
        src/planet/audio/audio.cpp
        src/lua/bindings/audio_bindings.cpp
    )
    set(AUDIO_DEFINES "")
endif()

set(ENGINE_SOURCES
    src/planet/core/engine.cpp
    src/planet/core/window.cpp
    src/planet/core/input.cpp
    src/planet/core/scene.cpp
    src/planet/core/camera.cpp
    src/planet/core/console.cpp
    src/planet/debug/debug_server.cpp
    src/planet/core/dexecl_engine.cpp
    src/planet/render/renderer.cpp
    src/planet/render/shader.cpp
    src/planet/render/mesh.cpp
    src/planet/render/texture.cpp
    src/planet/render/sprite_renderer.cpp
    src/planet/render/model_renderer.cpp
    src/planet/render/text_renderer.cpp
    src/planet/render/framebuffer.cpp
    src/planet/render/post_processor.cpp
    ${AUDIO_SOURCES}
    src/planet/ecs/ecs.cpp
    src/planet/ecs/component.cpp
    src/planet/ecs/system.cpp
    src/planet/physics/physics.cpp
    src/planet/resource/resource_manager.cpp
    src/planet/core/timer.cpp
    src/planet/core/random.cpp
    src/planet/core/math.cpp
    src/planet/core/json.cpp
    src/planet/map/map_loader.cpp
    src/planet/map/map_parser.cpp
    src/lua/lua_engine.cpp
    src/lua/bindings/core_bindings.cpp
    src/lua/bindings/render_bindings.cpp
    src/lua/bindings/input_bindings.cpp
    src/lua/bindings/ecs_bindings.cpp
    src/lua/bindings/physics_bindings.cpp
    src/lua/bindings/dexecl_bindings.cpp
    src/lua/bindings/map_bindings.cpp
    src/bundler/kerdata.cpp
    thirdparty/glad/src/glad.c
)

add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC thirdparty/lua/src)
target_compile_definitions(lua PRIVATE LUA_COMPAT_5_3)

if(UNIX AND NOT APPLE)
    target_link_libraries(lua PUBLIC m dl)
endif()

add_library(planet_lib STATIC ${ENGINE_SOURCES})
target_include_directories(planet_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glad/include
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/thirdparty/lua/src
    ${CMAKE_SOURCE_DIR}/thirdparty
)
target_compile_definitions(planet_lib PUBLIC GLFW_INCLUDE_NONE)

if(PLANET_USE_FMOD)
    target_compile_definitions(planet_lib PRIVATE PLANET_USE_FMOD)
    target_include_directories(planet_lib PRIVATE ${FMOD_INCLUDE_DIR})
    target_link_libraries(planet_lib PUBLIC
        lua OpenGL::GL glfw ${FMOD_LIBRARY} Threads::Threads
    )
    if(UNIX AND NOT APPLE)
        target_link_libraries(planet_lib PRIVATE pthread dl)
    endif()
elseif(PLANET_NO_AUDIO)
    target_compile_definitions(planet_lib PRIVATE PLANET_NO_AUDIO)
    target_link_libraries(planet_lib PUBLIC
        lua OpenGL::GL glfw Threads::Threads
    )
    if(UNIX AND NOT APPLE)
        target_link_libraries(planet_lib PRIVATE pthread dl)
    endif()
else()
    if(APPLE)
        target_link_libraries(planet_lib PUBLIC
            lua OpenGL::GL glfw "${OPENAL_LIBRARY}" Threads::Threads
        )
        if(OPENAL_INCLUDE_DIR)
            target_include_directories(planet_lib PRIVATE "${OPENAL_INCLUDE_DIR}")
        endif()
    else()
        target_link_libraries(planet_lib PUBLIC
            lua OpenGL::GL glfw OpenAL::OpenAL Threads::Threads
        )
    endif()
endif()

add_executable(planet src/main.cpp)
target_include_directories(planet PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glad/include
    ${CMAKE_SOURCE_DIR}/thirdparty
)

target_link_libraries(planet PRIVATE
    planet_lib
)

if(PLANET_BUILD_WINDOWS)
    set_target_properties(planet PROPERTIES
        WIN32_EXECUTABLE TRUE
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    add_custom_command(TARGET planet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "/usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll"
            "/usr/x86_64-w64-mingw32/bin/libstdc++-6.dll"
            "/usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll"
            "$<TARGET_FILE_DIR:planet>"
        COMMENT "Copying mingw runtime DLLs"
    )
else()
    set_target_properties(planet PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

file(GLOB PROJECT_DIRS RELATIVE "${CMAKE_SOURCE_DIR}/projects" "${CMAKE_SOURCE_DIR}/projects/*")
foreach(proj ${PROJECT_DIRS})
    file(COPY "${CMAKE_SOURCE_DIR}/projects/${proj}/" DESTINATION "${CMAKE_BINARY_DIR}/bin")
endforeach()
file(COPY fonts/ DESTINATION "${CMAKE_BINARY_DIR}/bin")

# Package target for Windows distribution
if(PLANET_BUILD_WINDOWS AND NOT TARGET package_windows)
    add_custom_target(package_windows
        COMMAND ${CMAKE_COMMAND} -E echo "Packaging Windows distribution..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/dist/planet-windows"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/planet.exe" "${CMAKE_BINARY_DIR}/dist/planet-windows/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libgcc_s_seh-1.dll" "${CMAKE_BINARY_DIR}/dist/planet-windows/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libstdc++-6.dll" "${CMAKE_BINARY_DIR}/dist/planet-windows/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/libwinpthread-1.dll" "${CMAKE_BINARY_DIR}/dist/planet-windows/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/bin" "${CMAKE_BINARY_DIR}/dist/planet-windows"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/fonts" "${CMAKE_BINARY_DIR}/dist/planet-windows/fonts"
        COMMAND ${CMAKE_COMMAND} -E tar cf "${CMAKE_BINARY_DIR}/dist/planet-windows.tar.gz" --format=gnutar "${CMAKE_BINARY_DIR}/dist/planet-windows"
        COMMENT "Creating Windows distribution package"
    )
endif()
