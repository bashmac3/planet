#include "planet/editor/project.h"
#include <fstream>
#include <sstream>

namespace planet {

static std::string EscapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }
    return out;
}

std::string ProjectManifest::Serialize(const ProjectManifest& m) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"name\": \"" << EscapeJson(m.name) << "\",\n";
    ss << "  \"version\": \"" << EscapeJson(m.version) << "\",\n";
    ss << "  \"mainScript\": \"" << EscapeJson(m.mainScript) << "\",\n";
    ss << "  \"libraries\": [";
    for (size_t i = 0; i < m.libraries.size(); i++) {
        if (i > 0) ss << ", ";
        ss << "\"" << EscapeJson(m.libraries[i]) << "\"";
    }
    ss << "],\n";
    ss << "  \"assetPaths\": [";
    for (size_t i = 0; i < m.assetPaths.size(); i++) {
        if (i > 0) ss << ", ";
        ss << "\"" << EscapeJson(m.assetPaths[i]) << "\"";
    }
    ss << "],\n";
    ss << "  \"buildOutput\": \"" << EscapeJson(m.buildOutput) << "\",\n";
    ss << "  \"crossCompileWindows\": " << (m.crossCompileWindows ? "true" : "false") << "\n";
    ss << "}\n";
    return ss.str();
}

ProjectManifest ProjectManifest::Deserialize(const std::string& json) {
    ProjectManifest m;
    auto findStr = [&](const std::string& key) -> std::string {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return {};
        pos = json.find('"', pos + key.size() + 3);
        if (pos == std::string::npos) return {};
        std::string val;
        pos++;
        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                pos++;
                if (json[pos] == 'n') val += '\n';
                else if (json[pos] == 't') val += '\t';
                else val += json[pos];
            } else {
                val += json[pos];
            }
            pos++;
        }
        return val;
    };
    auto findArr = [&](const std::string& key) -> std::vector<std::string> {
        std::vector<std::string> arr;
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return arr;
        pos = json.find('[', pos);
        if (pos == std::string::npos) return arr;
        pos++;
        while (pos < json.size() && json[pos] != ']') {
            while (pos < json.size() && json[pos] != '"' && json[pos] != ']') pos++;
            if (pos >= json.size() || json[pos] == ']') break;
            pos++;
            std::string val;
            while (pos < json.size() && json[pos] != '"') {
                if (json[pos] == '\\' && pos + 1 < json.size()) {
                    pos++;
                    val += json[pos];
                } else {
                    val += json[pos];
                }
                pos++;
            }
            arr.push_back(val);
            pos++;
        }
        return arr;
    };
    auto findBool = [&](const std::string& key, bool def) -> bool {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return def;
        auto tpos = json.find("true", pos);
        auto fpos = json.find("false", pos);
        if (tpos != std::string::npos && (fpos == std::string::npos || tpos < fpos)) return true;
        return false;
    };

    m.name = findStr("name");
    m.version = findStr("version");
    m.mainScript = findStr("mainScript");
    m.libraries = findArr("libraries");
    m.assetPaths = findArr("assetPaths");
    m.buildOutput = findStr("buildOutput");
    m.crossCompileWindows = findBool("crossCompileWindows", false);

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
