#include "bundler/bundler.h"
#include <iostream>
#include <cstring>

void printUsage(const char* prog) {
    std::cout << "Planet Engine Bundler v0.1.0\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  pack     - Pack scripts/assets into .kerdata archive\n";
    std::cout << "  bundle   - Generate platform application bundle\n";
    std::cout << "  info     - Print info about a .kerdata file\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -o PATH        Output path (default: ./out)\n";
    std::cout << "  -n NAME        Application name (default: planet)\n";
    std::cout << "  -v VERSION     Version string (default: 0.1.0)\n";
    std::cout << "  -s DIR         Script directory (repeatable)\n";
    std::cout << "  -a DIR         Asset directory (repeatable)\n";
    std::cout << "  -d DIR         Dexecl directory (repeatable)\n";
    std::cout << "  -m SCRIPT      Main script path (default: scripts/main.lua)\n";
    std::cout << "  -t TARGET      Bundle target: windows, macos, unix, auto\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << prog << " pack -s ./scripts -a ./assets -d ./dexecl -o ./game.kerdata\n";
    std::cout << "  " << prog << " bundle -s ./scripts -a ./assets -o ./dist -n MyGame\n";
    std::cout << "  " << prog << " info ./game.kerdata\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    planet::BundleConfig config;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            config.outputPath = argv[++i];
        } else if (arg == "-n" && i + 1 < argc) {
            config.appName = argv[++i];
        } else if (arg == "-v" && i + 1 < argc) {
            config.appVersion = argv[++i];
        } else if (arg == "-s" && i + 1 < argc) {
            config.scriptDirs.push_back(argv[++i]);
        } else if (arg == "-a" && i + 1 < argc) {
            config.assetDirs.push_back(argv[++i]);
        } else if (arg == "-d" && i + 1 < argc) {
            config.dexeclDirs.push_back(argv[++i]);
        } else if (arg == "-m" && i + 1 < argc) {
            config.mainScript = argv[++i];
        }
    }

    if (config.outputPath.empty()) config.outputPath = "./out";
    if (config.scriptDirs.empty()) config.scriptDirs.push_back("./scripts");
    if (config.assetDirs.empty())  config.assetDirs.push_back("./assets");
    if (config.dexeclDirs.empty()) config.dexeclDirs.push_back("./dexecl");

    if (command == "info") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " info <file.kerdata>" << std::endl;
            return 1;
        }
        planet::KerdataArchive archive;
        if (!planet::KerdataRead(archive, argv[2])) {
            std::cerr << "Failed to read: " << argv[2] << std::endl;
            return 1;
        }
        std::cout << "=== " << argv[2] << " ===" << std::endl;
        std::cout << "Files: " << archive.header.fileCount << std::endl;
        std::cout << "Data size: " << archive.header.totalDataSize << " bytes" << std::endl;
        for (const auto& e : archive.entries) {
            std::cout << "  " << e.path << " (" << e.dataSize << " bytes)" << std::endl;
        }
        return 0;
    }

    if (command == "pack") {
        planet::KerdataArchive archive;
        if (!planet::BuildKerdata(config, archive)) return 1;
        std::string out = config.outputPath;
        if (out.find(".kerdata") == std::string::npos) out += "/" + config.appName + ".kerdata";
        return planet::KerdataWrite(archive, out) ? 0 : 1;
    }

    if (command == "bundle") {
        return planet::Bundle(config) ? 0 : 1;
    }

    printUsage(argv[0]);
    return 1;
}
