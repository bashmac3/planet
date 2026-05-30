#include "planet/render/shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "planet/core/logger.h"

namespace planet {

Shader::Shader() {}

Shader::~Shader() {
    if (m_programID) {
        glDeleteProgram(m_programID);
    }
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    m_vertexPath = vertexPath;
    m_fragmentPath = fragmentPath;

    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        LOG_ERROR() << "[Shader] Failed to open shader files: " << vertexPath << ", " << fragmentPath;
        return false;
    }

    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    m_vertexSource = vStream.str();
    m_fragmentSource = fStream.str();

    return LoadFromSource(m_vertexSource, m_fragmentSource);
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (!vertex || !fragment) {
        return false;
    }

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertex);
    glAttachShader(m_programID, fragment);
    glLinkProgram(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        LOG_ERROR() << "[Shader] Program linking failed:\n" << infoLog;
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    m_uniformCache.clear();
    return true;
}

GLuint Shader::CompileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOG_ERROR() << "[Shader] Compilation failed (" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "):\n" << infoLog;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void Shader::Bind() const {
    glUseProgram(m_programID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

GLint Shader::GetUniformLocation(const std::string& name) const {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(m_programID, name.c_str());
    m_uniformCache[name] = location;
    return location;
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(GetUniformLocation(name), value ? 1 : 0);
}



bool Shader::Reload() {
    if (!m_vertexSource.empty() && !m_fragmentSource.empty()) {
        GLuint oldProg = m_programID;
        if (LoadFromSource(m_vertexSource, m_fragmentSource)) {
            if (oldProg) glDeleteProgram(oldProg);
            return true;
        }
    } else if (!m_vertexPath.empty() && !m_fragmentPath.empty()) {
        std::string vPath = m_vertexPath;
        std::string fPath = m_fragmentPath;
        GLuint oldProg = m_programID;
        bool ok = LoadFromFile(vPath, fPath);
        if (ok) {
            if (oldProg) glDeleteProgram(oldProg);
            return true;
        }
    }
    return false;
}

bool Shader::HasUniform(const std::string& name) const {
    return glGetUniformLocation(m_programID, name.c_str()) != -1;
}

} // namespace planet
