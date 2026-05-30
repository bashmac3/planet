#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/core/camera.h"
#include "planet/core/json.h"
#include "planet/render/mesh.h"
#include "planet/render/texture.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace planet {

Scene& Scene::Instance() {
    static Scene instance;
    return instance;
}

void Scene::Load() {
    m_activeCamera = std::make_unique<Camera>();
}

void Scene::Unload() {
    m_entities.clear();
    m_activeCamera.reset();
}

void Scene::Update(double dt) {
    for (auto& entity : m_entities) {
        entity->Update(dt);
    }
}

void Scene::Clear() {
    m_entities.clear();
}

Entity* Scene::CreateEntity(const std::string& name) {
    auto entity = std::make_unique<Entity>(name);
    Entity* ptr = entity.get();
    m_entities.push_back(std::move(entity));
    return ptr;
}

static glm::vec3 ParseVec3(const json::Value& v) {
    return glm::vec3(v[0].asFloat(), v[1].asFloat(), v[2].asFloat());
}

static glm::vec4 ParseVec4(const json::Value& v) {
    return glm::vec4(v[0].asFloat(), v[1].asFloat(), v[2].asFloat(), v[3].asFloat());
}

static Entity* CreateEntityFromJson(Scene& scene, const json::Value& root);

static void ApplyTransformFromJson(Entity* entity, const json::Value& val) {
    if (val.has("position")) entity->SetPosition(ParseVec3(val["position"]));
    if (val.has("rotation")) {
        auto& r = val["rotation"];
        if (r.size() == 4)
            entity->SetRotation(glm::quat(r[0].asFloat(), r[1].asFloat(), r[2].asFloat(), r[3].asFloat()));
    }
    if (val.has("scale")) entity->SetScale(ParseVec3(val["scale"]));
    if (val.has("euler")) {
        auto e = ParseVec3(val["euler"]);
        entity->SetEulerAngles(glm::vec3(glm::radians(e.x), glm::radians(e.y), glm::radians(e.z)));
    }
}

static Entity* CreateEntityFromJson(Scene& scene, const json::Value& root) {
    if (!root.isObject() || !root.has("name")) return nullptr;

    Entity* entity = scene.CreateEntity(root["name"].asString());

    if (root.has("tags")) {
        for (auto& tag : root["tags"].arr) {
            entity->AddTag(tag.asString());
        }
    }

    ApplyTransformFromJson(entity, root);

    if (root.has("components")) {
        for (auto& comp : root["components"].arr) {
            if (!comp.has("type")) continue;
            std::string type = comp["type"].asString();

            if (type == "MeshComponent") {
                auto* mc = entity->AddComponent<MeshComponent>();
                if (comp.has("color")) mc->color = ParseVec4(comp["color"]);
                if (comp.has("wireframe")) mc->wireframe = comp["wireframe"].asBool();
                if (comp.has("visible")) mc->visible = comp["visible"].asBool();
                if (comp.has("unlit")) mc->unlit = comp["unlit"].asBool();
                if (comp.has("mesh")) {
                    std::string meshStr = comp["mesh"].asString();
                    if (meshStr == "cube") mc->mesh = new Mesh(Mesh::CreateCube());
                    else if (meshStr == "sphere") mc->mesh = new Mesh(Mesh::CreateSphere());
                    else if (meshStr == "plane") mc->mesh = new Mesh(Mesh::CreatePlane());
                    else if (meshStr == "quad") mc->mesh = new Mesh(Mesh::CreateQuad());
                    else if (meshStr == "cylinder") mc->mesh = new Mesh(Mesh::CreateCylinder());
                    else if (meshStr == "cone") mc->mesh = new Mesh(Mesh::CreateCone());
                    else if (meshStr == "torus") mc->mesh = new Mesh(Mesh::CreateTorus());
                    else if (meshStr == "icosahedron") mc->mesh = new Mesh(Mesh::CreateIcosahedron());
                    else if (meshStr == "grid") {
                        float gw = comp.has("width") ? comp["width"].asFloat() : 1.0f;
                        float gd = comp.has("depth") ? comp["depth"].asFloat() : 1.0f;
                        int gdiv = comp.has("divisions") ? comp["divisions"].asInt() : 10;
                        mc->mesh = new Mesh(Mesh::CreateGrid(gw, gd, gdiv));
                    }
                    else if (meshStr == "circle") mc->mesh = new Mesh(Mesh::CreateCircle());
                    else if (meshStr == "tetrahedron") mc->mesh = new Mesh(Mesh::CreateTetrahedron());
                    else if (meshStr == "octahedron") mc->mesh = new Mesh(Mesh::CreateOctahedron());
                    else if (meshStr == "box") {
                        float w = comp.has("width") ? comp["width"].asFloat() : 1.0f;
                        float h = comp.has("height") ? comp["height"].asFloat() : 1.0f;
                        float d = comp.has("depth") ? comp["depth"].asFloat() : 1.0f;
                        mc->mesh = new Mesh(Mesh::CreateBox(w, h, d));
                    }
                }
                if (comp.has("texture")) {
                    auto tex = new Texture();
                    if (tex->LoadFromFile(comp["texture"].asString()))
                        mc->texture = tex;
                    else
                        delete tex;
                }
            } else if (type == "SpriteComponent") {
                auto* sc = entity->AddComponent<SpriteComponent>();
                if (comp.has("size")) sc->size = glm::vec2(comp["size"][0].asFloat(), comp["size"][1].asFloat());
                if (comp.has("color")) sc->color = ParseVec4(comp["color"]);
                if (comp.has("rotation")) sc->rotation = comp["rotation"].asFloat();
                if (comp.has("layer")) sc->layer = comp["layer"].asInt();
                if (comp.has("visible")) sc->visible = comp["visible"].asBool();
                if (comp.has("texture")) {
                    auto tex = new Texture();
                    if (tex->LoadFromFile(comp["texture"].asString()))
                        sc->texture = tex;
                    else
                        delete tex;
                }
            } else if (type == "ScriptComponent") {
                auto* sc = entity->AddComponent<ScriptComponent>();
                if (comp.has("script")) sc->scriptPath = comp["script"].asString();
            } else if (type == "LightComponent") {
                auto* lc = entity->AddComponent<LightComponent>();
                if (comp.has("color")) lc->color = ParseVec3(comp["color"]);
                if (comp.has("intensity")) lc->intensity = comp["intensity"].asFloat();
                if (comp.has("range")) lc->range = comp["range"].asFloat();
                if (comp.has("spotAngle")) lc->spotAngle = comp["spotAngle"].asFloat();
                if (comp.has("lightType")) {
                    std::string lt = comp["lightType"].asString();
                    if (lt == "directional") lc->type = LightComponent::Directional;
                    else if (lt == "point") lc->type = LightComponent::Point;
                    else if (lt == "spot") lc->type = LightComponent::Spot;
                }
            } else if (type == "CameraComponent") {
                auto* cc = entity->AddComponent<CameraComponent>();
                if (comp.has("fov")) cc->fov = comp["fov"].asFloat();
                if (comp.has("near")) cc->nearPlane = comp["near"].asFloat();
                if (comp.has("far")) cc->farPlane = comp["far"].asFloat();
                if (comp.has("isOrthographic")) cc->isOrthographic = comp["isOrthographic"].asBool();
                if (comp.has("orthoSize")) cc->orthoSize = comp["orthoSize"].asFloat();
                if (comp.has("isMain")) cc->isMain = comp["isMain"].asBool();
            } else if (type == "RigidbodyComponent") {
                auto* rb = entity->AddComponent<RigidbodyComponent>();
                if (comp.has("mass")) rb->mass = comp["mass"].asFloat();
                if (comp.has("useGravity")) rb->useGravity = comp["useGravity"].asBool();
                if (comp.has("isKinematic")) rb->isKinematic = comp["isKinematic"].asBool();
                if (comp.has("drag")) rb->drag = comp["drag"].asFloat();
            } else if (type == "ColliderComponent") {
                auto* cc = entity->AddComponent<ColliderComponent>();
                if (comp.has("size")) cc->size = ParseVec3(comp["size"]);
                if (comp.has("center")) cc->center = ParseVec3(comp["center"]);
                if (comp.has("isTrigger")) cc->isTrigger = comp["isTrigger"].asBool();
                if (comp.has("colliderType")) {
                    std::string ct = comp["colliderType"].asString();
                    if (ct == "box") cc->type = ColliderComponent::Box;
                    else if (ct == "sphere") cc->type = ColliderComponent::Sphere;
                    else if (ct == "capsule") cc->type = ColliderComponent::Capsule;
                    else if (ct == "mesh") cc->type = ColliderComponent::Mesh;
                }
            } else if (type == "AudioSourceComponent") {
                auto* ac = entity->AddComponent<AudioSourceComponent>();
                if (comp.has("sound")) ac->soundPath = comp["sound"].asString();
                if (comp.has("volume")) ac->volume = comp["volume"].asFloat();
                if (comp.has("pitch")) ac->pitch = comp["pitch"].asFloat();
                if (comp.has("loop")) ac->loop = comp["loop"].asBool();
                if (comp.has("playOnStart")) ac->playOnStart = comp["playOnStart"].asBool();
                if (comp.has("minDistance")) ac->minDistance = comp["minDistance"].asFloat();
                if (comp.has("maxDistance")) ac->maxDistance = comp["maxDistance"].asFloat();
                if (comp.has("is3D")) ac->is3D = comp["is3D"].asBool();
            } else if (type == "BillboardComponent") {
                auto* bc = entity->AddComponent<BillboardComponent>();
                if (comp.has("size")) bc->size = glm::vec2(comp["size"][0].asFloat(), comp["size"][1].asFloat());
                if (comp.has("color")) bc->color = ParseVec4(comp["color"]);
                if (comp.has("lockY")) bc->lockY = comp["lockY"].asBool();
                if (comp.has("texture")) {
                    auto tex = new Texture();
                    if (tex->LoadFromFile(comp["texture"].asString()))
                        bc->texture = tex;
                    else
                        delete tex;
                }
            } else if (type == "TriggerComponent") {
                auto* tc = entity->AddComponent<TriggerComponent>();
                if (comp.has("size")) tc->size = ParseVec3(comp["size"]);
                if (comp.has("oneShot")) tc->oneShot = comp["oneShot"].asBool();
                if (comp.has("onEnter")) tc->onEnterScript = comp["onEnter"].asString();
                if (comp.has("onExit")) tc->onExitScript = comp["onExit"].asString();
            } else if (type == "ParticleEmitterComponent") {
                auto* pc = entity->AddComponent<ParticleEmitterComponent>();
                if (comp.has("maxParticles")) pc->maxParticles = comp["maxParticles"].asInt();
                if (comp.has("emissionRate")) pc->emissionRate = comp["emissionRate"].asFloat();
                if (comp.has("lifetime")) pc->lifetime = comp["lifetime"].asFloat();
                if (comp.has("speed")) pc->speed = comp["speed"].asFloat();
                if (comp.has("startColor")) pc->startColor = ParseVec4(comp["startColor"]);
                if (comp.has("endColor")) pc->endColor = ParseVec4(comp["endColor"]);
                if (comp.has("startSize")) pc->startSize = comp["startSize"].asFloat();
                if (comp.has("endSize")) pc->endSize = comp["endSize"].asFloat();
                if (comp.has("gravityModifier")) pc->gravityModifier = ParseVec3(comp["gravityModifier"]);
                if (comp.has("looping")) pc->looping = comp["looping"].asBool();
                if (comp.has("playing")) pc->playing = comp["playing"].asBool();
            }
        }
    }

    if (root.has("children")) {
        for (auto& child : root["children"].arr) {
            Entity* childEntity = CreateEntityFromJson(scene, child);
            if (childEntity) {
                childEntity->SetParent(entity);
            }
        }
    }

    return entity;
}

Entity* Scene::CreateEntityFromPrefab(const std::string& prefabPath) {
    std::ifstream file(prefabPath);
    if (!file.is_open()) return nullptr;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto root = json::parse(content);
    return CreateEntityFromJson(*this, root);
}

void Scene::DestroyEntity(Entity* entity) {
    auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; });
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
}

Entity* Scene::FindEntity(const std::string& name) const {
    for (const auto& entity : m_entities) {
        if (entity->GetName() == name) {
            return entity.get();
        }
    }
    return nullptr;
}

std::vector<Entity*> Scene::GetEntitiesWithTag(const std::string& tag) const {
    std::vector<Entity*> result;
    for (const auto& entity : m_entities) {
        if (entity->HasTag(tag)) {
            result.push_back(entity.get());
        }
    }
    return result;
}

std::vector<Entity*> Scene::FindEntitiesByName(const std::string& name) const {
    std::vector<Entity*> result;
    for (const auto& entity : m_entities) {
        if (entity->GetName() == name) {
            result.push_back(entity.get());
        }
    }
    return result;
}

void Scene::DestroyAllWithTag(const std::string& tag) {
    auto it = m_entities.begin();
    while (it != m_entities.end()) {
        if ((*it)->HasTag(tag)) {
            it = m_entities.erase(it);
        } else {
            ++it;
        }
    }
}

Entity* Scene::GetEntityAtIndex(size_t index) const {
    if (index < m_entities.size()) {
        return m_entities[index].get();
    }
    return nullptr;
}

void Scene::SetAmbientLight(const glm::vec3& color) {
    m_ambientLight = color;
}

} // namespace planet
