#include "bundler/bundler.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "planet/core/logger.h"

namespace fs = std::filesystem;

namespace planet {

bool BuildKerdata(const BundleConfig& config, KerdataArchive& outArchive) {
    outArchive.entries.clear();
    outArchive.pathIndex.clear();
    outArchive.rawData.clear();

    std::string manifest = "app:" + config.appName + "\n" + "version:" + config.appVersion + "\n" + "main:" + config.mainScript + "\n";
    std::vector<uint8_t> manifestData(manifest.begin(), manifest.end());

    uint32_t dataCursor = 0;

    // Add manifest first
    {
        KerdataEntry e;
        e.path = "manifest.ini";
        e.dataOffset = dataCursor;
        e.dataSize   = static_cast<uint32_t>(manifestData.size());
        e.flags      = 0;
        outArchive.rawData.insert(outArchive.rawData.end(), manifestData.begin(), manifestData.end());
        dataCursor += e.dataSize;
        outArchive.entries.push_back(e);
        outArchive.pathIndex[e.path] = static_cast<uint32_t>(outArchive.entries.size() - 1);
    }

    // Load all collected files
    std::vector<std::string> dirs;
    for (const auto& d : config.scriptDirs) dirs.push_back(d);
    for (const auto& d : config.assetDirs)  dirs.push_back(d);
    for (const auto& d : config.dexeclDirs) dirs.push_back(d);

    for (size_t di = 0; di < dirs.size(); ++di) {
        const auto& dir = dirs[di];
        std::string prefix;
        if (di < config.scriptDirs.size()) prefix = "scripts";
        else if (di < config.scriptDirs.size() + config.assetDirs.size()) prefix = "assets";
        else prefix = "dexecl";

        if (!fs::exists(dir)) continue;
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            // skip hidden files
            std::string fname = entry.path().filename().string();
            if (!fname.empty() && fname[0] == '.') continue;

            std::string rel = fs::relative(entry.path(), dir).string();
            std::replace(rel.begin(), rel.end(), '\\', '/');

            std::ifstream infile(entry.path(), std::ios::binary | std::ios::ate);
            if (!infile.is_open()) continue;
            size_t fsize = infile.tellg();
            infile.seekg(0, std::ios::beg);
            std::vector<uint8_t> fdata(fsize);
            infile.read(reinterpret_cast<char*>(fdata.data()), fsize);
            infile.close();

            KerdataEntry e;
            e.path       = prefix + "/" + rel;
            e.dataOffset = dataCursor;
            e.dataSize   = static_cast<uint32_t>(fsize);
            e.flags      = 0;

            LOG_INFO() << "[Bundler] Packing " << fname << "...";

            outArchive.rawData.insert(outArchive.rawData.end(), fdata.begin(), fdata.end());
            dataCursor += e.dataSize;
            outArchive.entries.push_back(e);
            outArchive.pathIndex[e.path] = static_cast<uint32_t>(outArchive.entries.size() - 1);
        }
    }

    LOG_INFO() << "[Bundler] Packed " << outArchive.entries.size() << " files (" << dataCursor << " bytes)";
    return true;
}

bool GenerateWindowsBundle(const BundleConfig& config, const KerdataArchive& archive) {
    std::string outputPath = config.outputPath + "/" + config.appName + ".exe";
    std::string kerdataPath = config.outputPath + "/" + config.appName + ".kerdata";

    // Write .kerdata
    KerdataWrite(archive, kerdataPath);

    // On Windows, the .kerdata gets embedded at link time or appended to the .exe.
    // For now, we write the .kerdata as a sidecar next to the .exe.
    // Full embedding requires a stub EXE with a section appended.

    LOG_INFO() << "[Bundler] Windows bundle: " << outputPath;
    LOG_INFO() << "[Bundler]   Sidecar: " << kerdataPath;
    LOG_INFO() << "[Bundler]   (Embedded .exe requires MSVC/mingw link step)";
    return true;
}

bool GenerateMacOSBundle(const BundleConfig& config, const KerdataArchive& archive) {
    std::string appDir = config.outputPath + "/" + config.appName + ".app";
    std::string contentsDir = appDir + "/Contents";
    std::string macosDir = contentsDir + "/MacOS";
    std::string resourcesDir = contentsDir + "/Resources";

    fs::create_directories(macosDir);
    fs::create_directories(resourcesDir);

    // Write .kerdata into Resources
    std::string kerdataPath = resourcesDir + "/" + config.appName + ".kerdata";
    KerdataWrite(archive, kerdataPath);

    // Info.plist
    std::string plist = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>)" + config.appName + R"(</string>
    <key>CFBundleIdentifier</key>
    <string>com.planet.engine</string>
    <key>CFBundleName</key>
    <string>)" + config.appName + R"(</string>
    <key>CFBundleVersion</key>
    <string>)" + config.appVersion + R"(</string>
    <key>CFBundleShortVersionString</key>
    <string>)" + config.appVersion + R"(</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>)";
    std::ofstream plistFile(contentsDir + "/Info.plist");
    plistFile << plist;
    plistFile.close();

    LOG_INFO() << "[Bundler] macOS .app bundle: " << appDir;
    return true;
}

bool GenerateUnixBundle(const BundleConfig& config, const KerdataArchive& archive) {
    std::string outputPath = config.outputPath + "/" + config.appName;
    std::string kerdataPath = outputPath + ".kerdata";

    KerdataWrite(archive, kerdataPath);

    // Create launcher script
    std::string launcher = outputPath + ".sh";
    std::ofstream lf(launcher);
    lf << "#!/bin/sh\n";
    lf << "# Planet Engine Launcher\n";
    lf << "DIR=\"$(cd \"$(dirname \"$0\")\" && pwd)\"\n";
    lf << "exec \"$DIR/" << config.appName << "\" \"$DIR/" << config.appName << ".kerdata\"\n";
    lf.close();
    fs::permissions(launcher, fs::perms::owner_exec | fs::perms::group_exec |
                               fs::perms::others_exec, fs::perm_options::add);

    LOG_INFO() << "[Bundler] Unix bundle: " << outputPath;
    LOG_INFO() << "[Bundler]   Data: " << kerdataPath;
    LOG_INFO() << "[Bundler]   Launcher: " << launcher;
    return true;
}

bool Bundle(const BundleConfig& config) {
    KerdataArchive archive;
    if (!BuildKerdata(config, archive)) {
        LOG_ERROR() << "[Bundler] Failed to build .kerdata";
        return false;
    }

#ifdef _WIN32
    return GenerateWindowsBundle(config, archive);
#elif defined(__APPLE__)
    return GenerateMacOSBundle(config, archive);
#else
    return GenerateUnixBundle(config, archive);
#endif
}

} // namespace planet
