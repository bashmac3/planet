#include "planet/resource/resource_manager.h"
#include "planet/render/texture.h"
#include "planet/render/mesh.h"
#include "planet/render/shader.h"
#include "bundler/kerdata.h"
#include "planet/core/logger.h"

namespace planet {

ResourceManager& ResourceManager::Instance() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::Init(const std::string& assetPath) {
    m_assetPath = assetPath;
    LOG_INFO() << "[Resource] Asset path: " << m_assetPath;
}

void ResourceManager::LoadKerdata(const std::string& path) {
    m_kerdata = std::make_unique<KerdataArchive>();
    if (KerdataRead(*m_kerdata, path)) {
        LOG_INFO() << "[Resource] Loaded kerdata: " << path + " (" << m_kerdata->header.fileCount + " files)";
    } else {
        LOG_ERROR() << "[Resource] Failed to load kerdata: " << path;
        m_kerdata.reset();
    }
}

bool ResourceManager::ReadKerdataFile(const std::string& path, std::vector<uint8_t>& outData) const {
    if (!m_kerdata) return false;
    const uint8_t* data = nullptr;
    size_t size = 0;
    if (!KerdataGetFile(*m_kerdata, path, &data, &size)) return false;
    outData.assign(data, data + size);
    return true;
}

void ResourceManager::Shutdown() {
    m_textures.clear();
    m_meshes.clear();
    m_shaders.clear();
    LOG_INFO() << "[Resource] Resources freed.";
}

std::string ResourceManager::GetFullPath(const std::string& relativePath) const {
    if (relativePath.empty()) return m_assetPath;
    if (relativePath[0] == '/') return relativePath;
    return m_assetPath + relativePath;
}

Texture* ResourceManager::LoadTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) return it->second.get();

    auto texture = std::make_unique<Texture>();
    std::string fullPath = GetFullPath(path);
    if (!texture->LoadFromFile(fullPath)) return nullptr;

    Texture* ptr = texture.get();
    m_textures[path] = std::move(texture);
    return ptr;
}

Mesh* ResourceManager::LoadMesh(const std::string& path) {
    auto it = m_meshes.find(path);
    if (it != m_meshes.end()) return it->second.get();

    return nullptr;
}

Shader* ResourceManager::LoadShader(const std::string& name,
                                     const std::string& vertPath,
                                     const std::string& fragPath) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) return it->second.get();

    auto shader = std::make_unique<Shader>();
    if (!shader->LoadFromFile(GetFullPath(vertPath), GetFullPath(fragPath))) {
        return nullptr;
    }

    Shader* ptr = shader.get();
    m_shaders[name] = std::move(shader);
    return ptr;
}

Texture* ResourceManager::GetTexture(const std::string& path) const {
    auto it = m_textures.find(path);
    return (it != m_textures.end()) ? it->second.get() : nullptr;
}

Mesh* ResourceManager::GetMesh(const std::string& path) const {
    auto it = m_meshes.find(path);
    return (it != m_meshes.end()) ? it->second.get() : nullptr;
}

Shader* ResourceManager::GetShader(const std::string& name) const {
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second.get() : nullptr;
}

} // namespace planet
