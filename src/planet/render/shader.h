#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace planet {

class Shader {
public:
    Shader();
    ~Shader();

    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

    void Bind() const;
    void Unbind() const;

    GLuint GetID() const { return m_programID; }

    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;
    void SetBool(const std::string& name, bool value) const;

    bool Reload();
    bool IsValid() const { return m_programID != 0; }
    bool HasUniform(const std::string& name) const;

    std::string GetVertexPath() const { return m_vertexPath; }
    std::string GetFragmentPath() const { return m_fragmentPath; }

private:
    GLuint CompileShader(GLenum type, const std::string& source);
    GLint GetUniformLocation(const std::string& name) const;

    GLuint m_programID = 0;
    mutable std::unordered_map<std::string, GLint> m_uniformCache;
    std::string m_vertexPath;
    std::string m_fragmentPath;
    std::string m_vertexSource;
    std::string m_fragmentSource;
};

} // namespace planet
