#include "planet/map/map_parser.h"
#include "planet/map/map_loader.h"
#include "planet/core/scene.h"
#include "planet/core/camera.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/render/mesh.h"
#include "planet/render/texture.h"
#include "planet/resource/resource_manager.h"
#include "planet/core/logger.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <memory>

namespace planet {

// ── Internal helpers ───────────────────────────────────────────

static std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t')) a++;
    while (b > a && (s[b-1] == ' ' || s[b-1] == '\t')) b--;
    return s.substr(a, b - a);
}

static std::vector<float> parseFloats(const std::string& s) {
    std::vector<float> out;
    std::string cur;
    for (char c : s) {
        if (c == ',') { if (!cur.empty()) out.push_back(std::stof(cur)); cur.clear(); }
        else cur += c;
    }
    if (!cur.empty()) out.push_back(std::stof(cur));
    return out;
}

// Shared mesh cache for quick access
static std::unordered_map<std::string, std::unique_ptr<Mesh>> s_meshes;

static Mesh* getMesh(const std::string& type) {
    auto it = s_meshes.find(type);
    if (it != s_meshes.end()) return it->second.get();
    Mesh* m = nullptr;
    if (type == "Cube" || type == "cube" || type == "Box" || type == "box")
        m = new Mesh(Mesh::CreateCube(1.0f));
    else if (type == "Sphere" || type == "sphere")
        m = new Mesh(Mesh::CreateSphere(0.5f, 16));
    else if (type == "Plane" || type == "plane")
        m = new Mesh(Mesh::CreatePlane(1, 1));
    else if (type == "Cylinder" || type == "cylinder")
        m = new Mesh(Mesh::CreateCylinder(0.5f, 1.0f, 16));
    else if (type == "Cone" || type == "cone")
        m = new Mesh(Mesh::CreateCone(0.5f, 1.0f, 16));
    else if (type == "Torus" || type == "torus")
        m = new Mesh(Mesh::CreateTorus(0.5f, 0.2f, 16, 8));
    if (m) { s_meshes[type] = std::unique_ptr<Mesh>(m); return m; }
    return nullptr;
}

// ── Line parser ────────────────────────────────────────────────
//
//  ["name";flag1=val1,...]:[MeshType](PROPERTY) = val1,val2,...

struct ParsedLine {
    std::string name;       // empty for _none
    std::unordered_map<std::string, std::string> flags;
    std::string meshType;   // empty if no mesh
    std::string prop;       // property name (e.g. SCALE, COLOR)
    std::string rawValue;   // everything after '='
    bool valid = false;
};

static ParsedLine parseLine(const std::string& line) {
    ParsedLine out;
    if (line.empty() || line[0] == '#' || line[0] == '/' || line[0] == ';')
        return out;

    // Find the name bracket
    size_t p = line.find('[');
    if (p == std::string::npos) return out;
    size_t q = line.find(']', p);
    if (q == std::string::npos) return out;

    std::string bracket = line.substr(p + 1, q - p - 1);
    // bracket = "name;flag1=val1,..."
    size_t semi = bracket.find(';');
    std::string namePart = (semi == std::string::npos) ? bracket : bracket.substr(0, semi);
    out.name = trim(namePart);
    if (out.name == "_none") out.name.clear();

    // Parse flags after semicolon
    if (semi != std::string::npos) {
        std::string flagsStr = bracket.substr(semi + 1);
        std::stringstream ss(flagsStr);
        std::string item;
        while (std::getline(ss, item, ',')) {
            size_t eq = item.find('=');
            if (eq != std::string::npos)
                out.flags[trim(item.substr(0, eq))] = trim(item.substr(eq + 1));
            else
                out.flags[trim(item)] = "";
        }
    }

    // Find mesh bracket after ']:'
    size_t colon = line.find(':', q);
    if (colon == std::string::npos) return out;
    p = line.find('[', colon);
    if (p == std::string::npos) return out;
    q = line.find(']', p);
    if (q == std::string::npos) return out;
    out.meshType = trim(line.substr(p + 1, q - p - 1));

    // Find property in parentheses
    p = line.find('(', q);
    if (p == std::string::npos) return out;
    q = line.find(')', p);
    if (q == std::string::npos) return out;
    out.prop = trim(line.substr(p + 1, q - p - 1));

    // Find value after '='
    size_t eq = line.find('=', q);
    if (eq != std::string::npos)
        out.rawValue = trim(line.substr(eq + 1));

    out.valid = true;
    return out;
}

// ── Object accumulator ─────────────────────────────────────────
//
// Since one object can span multiple lines (same name),
// we accumulate properties until the name changes.

struct ObjectAccum {
    std::string name;
    std::unordered_map<std::string, std::string> flags;
    std::string meshType;
    struct Prop { std::string key; std::vector<float> nums; std::string str; };
    std::vector<Prop> props;
    bool hasMesh = false;
    bool hasLight = false;
    std::string lightType;
};

static void flushObject(ObjectAccum& acc) {
    if (acc.name.empty() && acc.props.empty() && !acc.hasMesh && !acc.hasLight)
        return;

    auto& scene = Scene::Instance();
    std::string entName = acc.name.empty() ? "env_obj" : acc.name;

    Entity* ent = scene.CreateEntity(entName);

    // Mesh component
    if (acc.hasMesh && !acc.meshType.empty()) {
        auto* mc = ent->AddComponent<MeshComponent>();
        mc->mesh = getMesh(acc.meshType);
        mc->color = glm::vec4(1, 1, 1, 1);
        mc->visible = true;

        for (auto& p : acc.props) {
            if (p.key == "COLOR" && p.nums.size() >= 3) {
                mc->color.r = p.nums[0];
                mc->color.g = p.nums[1];
                mc->color.b = p.nums[2];
                if (p.nums.size() >= 4) mc->color.a = p.nums[3];
            }
            if (p.key == "TEXTURE" && !p.str.empty()) {
                auto* tex = ResourceManager::Instance().LoadTexture(p.str);
                if (tex) mc->texture = tex;
            }
            if (p.key == "SCALE" && p.nums.size() >= 3)
                ent->SetScale(glm::vec3(p.nums[0], p.nums[1], p.nums[2]));
            if (p.key == "POSITION" && p.nums.size() >= 3)
                ent->SetPosition(glm::vec3(p.nums[0], p.nums[1], p.nums[2]));
            if (p.key == "ROTATION" && p.nums.size() >= 3)
                ent->SetEulerAngles(glm::vec3(p.nums[0], p.nums[1], p.nums[2]));
        }
    }

    // Light component
    if (acc.hasLight) {
        auto* lc = ent->AddComponent<LightComponent>();
        if (acc.lightType == "DIRECTIONAL") lc->type = LightComponent::Directional;
        else if (acc.lightType == "SPOT") lc->type = LightComponent::Spot;
        else lc->type = LightComponent::Point;

        for (auto& p : acc.props) {
            if (p.key == "COLOR" && p.nums.size() >= 3)
                lc->color = glm::vec3(p.nums[0], p.nums[1], p.nums[2]);
            if (p.key == "RANGE" && p.nums.size() >= 1)
                lc->range = p.nums[0];
            if (p.key == "ROTATION" && p.nums.size() >= 3)
                ent->SetEulerAngles(glm::vec3(p.nums[0], p.nums[1], p.nums[2]));
        }
    }

    // Register as template if named
    if (!acc.name.empty()) {
        MapTemplate tmpl;
        tmpl.isMesh = acc.hasMesh;
        tmpl.isLight = acc.hasLight;
        tmpl.meshType = acc.meshType;
        tmpl.lightType = acc.lightType;
        tmpl.scale = ent->GetScale();
        if (acc.hasMesh) {
            auto* mc = ent->GetComponent<MeshComponent>();
            if (mc) tmpl.color = mc->color;
        }
        if (acc.hasLight) {
            auto* lc = ent->GetComponent<LightComponent>();
            if (lc) {
                tmpl.lightColor = lc->color;
                tmpl.lightRange = lc->range;
            }
        }
        MapLoader::Instance().SetTemplate(acc.name, tmpl);
    }

    // Reset
    acc = ObjectAccum{};
}

// ── Public API ─────────────────────────────────────────────────

bool ParseMapFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        LOG_ERROR() << "[MapParser] Cannot open: " << path;
        return false;
    }

    ObjectAccum acc;
    std::string line;
    while (std::getline(f, line)) {
        ParsedLine pl = parseLine(line);
        if (!pl.valid) continue;

        // If name changed, flush previous object
        if (pl.name != acc.name) {
            flushObject(acc);
            acc.name = pl.name;
            acc.flags = pl.flags;
        }

        acc.meshType = pl.meshType;

        // Check for light type in property
        if (pl.prop == "LIGHT_DIRECTIONAL" || pl.prop == "LIGHT_POINT" || pl.prop == "LIGHT_SPOT") {
            acc.hasLight = true;
            acc.lightType = pl.prop;
            continue;
        }

        // Check for mesh type in property
        if (pl.prop == "MESH" || (!acc.hasLight && !acc.hasMesh && !pl.meshType.empty())) {
            acc.hasMesh = true;
            continue;
        }

        // Accumulate property
        ObjectAccum::Prop p;
        p.key = pl.prop;
        if (pl.prop == "TEXTURE") {
            p.str = pl.rawValue;
        } else {
            p.nums = parseFloats(pl.rawValue);
        }
        acc.props.push_back(p);

        // Setting a mesh type property implies hasMesh
        if (!acc.hasMesh && !pl.meshType.empty() && pl.meshType != "_none") {
            acc.hasMesh = true;
            acc.meshType = pl.meshType;
        }

        // Light properties imply hasLight
        if (pl.prop == "LIGHT_COLOR" || pl.prop == "RANGE") {
            acc.hasLight = true;
        }
    }

    flushObject(acc);
    LOG_INFO() << "[MapParser] Loaded: " << path;
    return true;
}

} // namespace planet
