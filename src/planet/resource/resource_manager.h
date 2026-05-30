#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cstdint>

namespace planet {

class Texture;
class Mesh;
class Shader;
class KerdataArchive;

class ResourceManager {
public:
    static ResourceManager& Instance();

    void Init(const std::string& assetPath);
    void Shutdown();

    void LoadKerdata(const std::string& path);
    bool HasKerdata() const { return m_kerdata != nullptr; }
    bool ReadKerdataFile(const std::string& path, std::vector<uint8_t>& outData) const;

    Texture* LoadTexture(const std::string& path);
    Mesh* LoadMesh(const std::string& path);
    Shader* LoadShader(const std::string& name, const std::string& vertPath,
                       const std::string& fragPath);

    Texture* GetTexture(const std::string& path) const;
    Mesh* GetMesh(const std::string& path) const;
    Shader* GetShader(const std::string& name) const;

    std::string GetAssetPath() const { return m_assetPath; }
    std::string GetFullPath(const std::string& relativePath) const;

    bool Exists(const std::string& path) const;
    void UnloadTexture(const std::string& path);
    void UnloadMesh(const std::string& path);
    void UnloadShader(const std::string& name);
    void UnloadAll();

    std::vector<std::string> GetLoadedTexturePaths() const;
    std::vector<std::string> GetLoadedMeshPaths() const;
    std::vector<std::string> GetLoadedShaderNames() const;

    Shader* GetDefaultShader() const;

    static bool FileExists(const std::string& path);
    static std::string ReadFile(const std::string& path);

private:
    ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::string m_assetPath;
    std::unique_ptr<KerdataArchive> m_kerdata;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    Shader* m_defaultShader = nullptr;
};

} // namespace planet
