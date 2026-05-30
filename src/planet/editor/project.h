#pragma once

#include <string>
#include <vector>

namespace planet {

struct ProjectManifest {
    std::string name = "Untitled";
    std::string version = "1.0.0";
    std::string mainScript;
    std::vector<std::string> libraries;
    std::vector<std::string> assetPaths = {"assets/"};
    std::string buildOutput = "build/game";
    bool crossCompileWindows = false;

    std::string filePath;

    bool Load(const std::string& path);
    bool Save();
    bool SaveAs(const std::string& path);

    static std::string Serialize(const ProjectManifest& m);
    static ProjectManifest Deserialize(const std::string& json);
};

} // namespace planet
