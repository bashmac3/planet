#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace planet {

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};

    Vertex() = default;
    Vertex(const glm::vec3& pos, const glm::vec3& normal = glm::vec3(0.0f, 1.0f, 0.0f),
           const glm::vec2& tex = glm::vec2(0.0f))
        : Position(pos), Normal(normal), TexCoords(tex) {}
};

class Mesh {
public:
    Mesh();
    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
    ~Mesh();

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void Upload(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
    void Draw() const;
    void Destroy();

    GLuint GetVAO() const { return m_vao; }
    size_t GetIndexCount() const { return m_indexCount; }

    static Mesh CreateQuad(float width = 1.0f, float height = 1.0f);
    static Mesh CreateCube(float size = 1.0f);
    static Mesh CreateSphere(float radius = 0.5f, int segments = 16);
    static Mesh CreatePlane(float width = 1.0f, float depth = 1.0f);
    static Mesh CreateBox(float w, float h, float d);
    static Mesh CreateWallQuad(float width, float height);
    static Mesh CreateCylinder(float radius = 0.5f, float height = 1.0f, int segments = 16);
    static Mesh CreateCone(float radius = 0.5f, float height = 1.0f, int segments = 16);
    static Mesh CreateTorus(float radius = 0.5f, float tubeRadius = 0.2f, int segments = 16, int tubeSegments = 8);
    static Mesh CreateIcosahedron(float radius = 0.5f);
    static Mesh CreateGrid(float width, float depth, int divisions);
    static Mesh CreateCircle(float radius = 0.5f, int segments = 16);
    static Mesh CreateTetrahedron(float radius = 0.5f);
    static Mesh CreateOctahedron(float radius = 0.5f);

    size_t GetVertexCount() const { return m_vertexCount; }
    void SetVertexCount(size_t count) { m_vertexCount = count; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_indexCount = 0;
    size_t m_vertexCount = 0;
};

} // namespace planet
