#pragma once

#include "bundler/kerdata.h"
#include <string>
#include <vector>

namespace planet {

struct BundleConfig {
    std::string outputPath;           // output directory or file
    std::string appName = "planet";     // application name
    std::string appVersion = "0.1.0";
    std::string appIcon;              // path to icon (optional)
    std::vector<std::string> scriptDirs;   // dirs to scan for .lua
    std::vector<std::string> assetDirs;    // dirs to scan for assets
    std::vector<std::string> dexeclDirs;   // dirs to scan for .dexl
    std::string mainScript = "scripts/main.lua";
    bool compress = false;
};

// Build .kerdata from directories
bool BuildKerdata(const BundleConfig& config, KerdataArchive& outArchive);

// Generate a Windows .exe (self-extracting bundle)
bool GenerateWindowsBundle(const BundleConfig& config, const KerdataArchive& archive);

// Generate a macOS .app bundle
bool GenerateMacOSBundle(const BundleConfig& config, const KerdataArchive& archive);

// Generate a Unix standalone (executable + .kerdata sidecar)
bool GenerateUnixBundle(const BundleConfig& config, const KerdataArchive& archive);

// All-in-one: detect platform and build appropriate bundle
bool Bundle(const BundleConfig& config);

} // namespace planet
