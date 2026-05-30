#include "planet/core/engine.h"
#include "planet/core/logger.h"
#include <string>
#include <climits>
#include <cstdlib>
#include <cstring>

#if defined(PLANET_BUILD_WINDOWS)
#include <direct.h>
#else
#include <unistd.h>
#endif

int main(int argc, char* argv[]) {
    {
        char exePath[PATH_MAX];
#if defined(PLANET_BUILD_WINDOWS)
        if (_fullpath(exePath, argv[0], PATH_MAX)) {
            char* lastSlash = strrchr(exePath, '\\');
            if (lastSlash) { *lastSlash = '\0'; }
            lastSlash = strrchr(exePath, '/');
            if (lastSlash) { *lastSlash = '\0'; }
            _chdir(exePath);
        }
#else
        if (realpath(argv[0], exePath)) {
            char* lastSlash = strrchr(exePath, '/');
            if (lastSlash) {
                *lastSlash = '\0';
                chdir(exePath);
            }
        }
#endif
    }
    planet::EngineConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--term" || arg == "-t") {
            config.termOutput = true;
        } else if (arg == "--console" || arg == "-c") {
            config.consoleEnabled = true;
        } else if (arg.size() > 8 && arg.substr(arg.size() - 8) == ".kerdata") {
            config.kerdataPath = arg;
            config.scriptPath = "scripts/main.lua";
        } else if (arg.size() > 4 && arg.substr(arg.size() - 4) == ".lua") {
            config.scriptPath = arg;
        }
    }

    LOG_INFO() << "Planet Engine v0.2.0 starting";

    if (!config.kerdataPath.empty()) {
        LOG_INFO() << "Kerdata: " << config.kerdataPath;
    }

    if (!planet::Engine::Instance().Init(config)) {
        LOG_ERROR() << "Failed to initialize engine";
        return 1;
    }

    planet::Engine::Instance().Run();
    planet::Engine::Instance().Shutdown();

    LOG_INFO() << "Planet Engine stopped";
    return 0;
}
