#include "planet/editor/project.h"
#include <fstream>
#include <sstream>

namespace planet {

static std::string Escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            default: out += c;
        }
    }
    return out;
}

static std::string Unescape(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            i++;
            out += s[i];
        } else {
            out += s[i];
        }
    }
    return out;
}

static std::string Trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r')) start++;
    size_t end = s.size();
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\r')) end--;
    return s.substr(start, end - start);
}

static std::string Quote(const std::string& s) {
    return "\"" + Escape(s) + "\"";
}

std::string ProjectManifest::Serialize(const ProjectManifest& m) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "    name = " << Quote(m.name) << ",\n";
    ss << "    version = " << Quote(m.version) << ",\n";
    ss << "    main = " << Quote(m.mainScript) << ",\n";
    ss << "    assets = " << Quote(m.assetPaths.empty() ? "assets/" : m.assetPaths[0]) << ",\n";

    ss << "    libraries = {";
    for (size_t i = 0; i < m.libraries.size(); i++) {
        if (i > 0) ss << ", ";
        ss << Quote(m.libraries[i]);
    }
    ss << " },\n";

    ss << "    build_output = " << Quote(m.buildOutput) << ",\n";
    ss << "    cross_compile_windows = " << (m.crossCompileWindows ? "true" : "false") << "\n";
    ss << "}\n";
    return ss.str();
}

ProjectManifest ProjectManifest::Deserialize(const std::string& data) {
    ProjectManifest m;

    auto findStr = [&](const std::string& key) -> std::string {
        size_t pos = 0;
        while (true) {
            pos = data.find(key, pos);
            if (pos == std::string::npos) return {};
            // check it's a whole-word match: preceded by whitespace or start
            if (pos > 0 && data[pos-1] != ' ' && data[pos-1] != '\t' && data[pos-1] != '\n') {
                pos++;
                continue;
            }
            // skip key and look for =
            pos = data.find('=', pos + key.size());
            if (pos == std::string::npos) return {};
            pos++;
            // skip whitespace
            while (pos < data.size() && (data[pos] == ' ' || data[pos] == '\t')) pos++;
            if (pos >= data.size()) return {};
            if (data[pos] != '"') return {};
            pos++;
            std::string val;
            while (pos < data.size() && data[pos] != '"') {
                if (data[pos] == '\\' && pos + 1 < data.size()) {
                    pos++;
                    val += data[pos];
                } else {
                    val += data[pos];
                }
                pos++;
            }
            return val;
        }
    };

    auto findBool = [&](const std::string& key, bool def) -> bool {
        size_t pos = 0;
        while (true) {
            pos = data.find(key, pos);
            if (pos == std::string::npos) return def;
            if (pos > 0 && data[pos-1] != ' ' && data[pos-1] != '\t' && data[pos-1] != '\n') {
                pos++;
                continue;
            }
            // check for true/false after the key
            size_t eq = data.find('=', pos + key.size());
            if (eq == std::string::npos) return def;
            std::string rest = data.substr(eq + 1);
            if (rest.find("true") != std::string::npos) return true;
            return false;
        }
    };

    auto findArr = [&](const std::string& key) -> std::vector<std::string> {
        std::vector<std::string> arr;
        size_t pos = 0;
        while (true) {
            pos = data.find(key, pos);
            if (pos == std::string::npos) return arr;
            if (pos > 0 && data[pos-1] != ' ' && data[pos-1] != '\t' && data[pos-1] != '\n') {
                pos++;
                continue;
            }
            size_t eq = data.find('=', pos + key.size());
            if (eq == std::string::npos) return arr;
            size_t brace = data.find('{', eq);
            if (brace == std::string::npos) return arr;
            brace++;
            while (brace < data.size() && data[brace] != '}') {
                while (brace < data.size() && data[brace] != '"' && data[brace] != '}') brace++;
                if (brace >= data.size() || data[brace] == '}') break;
                brace++;
                std::string val;
                while (brace < data.size() && data[brace] != '"') {
                    if (data[brace] == '\\' && brace + 1 < data.size()) {
                        brace++;
                        val += data[brace];
                    } else {
                        val += data[brace];
                    }
                    brace++;
                }
                arr.push_back(val);
                brace++;
            }
            return arr;
        }
    };

    m.name = findStr("name");
    m.version = findStr("version");
    m.mainScript = findStr("main");

    std::string assets = findStr("assets");
    if (!assets.empty()) {
        m.assetPaths.clear();
        m.assetPaths.push_back(assets);
    }

    m.libraries = findArr("libraries");
    m.buildOutput = findStr("build_output");
    m.crossCompileWindows = findBool("cross_compile_windows", false);

    if (m.name.empty()) m.name = "Untitled";
    if (m.version.empty()) m.version = "1.0.0";
    if (m.assetPaths.empty()) m.assetPaths.push_back("assets/");
    if (m.buildOutput.empty()) m.buildOutput = "build/game";

    return m;
}

bool ProjectManifest::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::stringstream ss;
    ss << file.rdbuf();
    *this = Deserialize(ss.str());
    filePath = path;
    return true;
}

bool ProjectManifest::Save() {
    if (filePath.empty()) return false;
    return SaveAs(filePath);
}

bool ProjectManifest::SaveAs(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << Serialize(*this);
    filePath = path;
    return true;
}

} // namespace planet
