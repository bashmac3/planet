#include "planet/render/mesh.h"
#include <cmath>

namespace planet {

Mesh::Mesh() {}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
    Upload(vertices, indices);
}

Mesh::~Mesh() {
    Destroy();
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo),
      m_indexCount(other.m_indexCount) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ebo = other.m_ebo;
        m_indexCount = other.m_indexCount;
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
        other.m_indexCount = 0;
    }
    return *this;
}

void Mesh::Upload(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
    Destroy();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Color));

    glBindVertexArray(0);

    m_indexCount = indices.size();
    m_vertexCount = vertices.size();
}

void Mesh::Draw() const {
    if (m_vao && m_indexCount > 0) {
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}

void Mesh::Destroy() {
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_indexCount = 0;
}

Mesh Mesh::CreateQuad(float width, float height) {
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    std::vector<Vertex> vertices = {
        {{-hw,  hh, 0.0f}, {0,0,1}, {0,1}},
        {{-hw, -hh, 0.0f}, {0,0,1}, {0,0}},
        {{ hw, -hh, 0.0f}, {0,0,1}, {1,0}},
        {{ hw,  hh, 0.0f}, {0,0,1}, {1,1}},
    };

    std::vector<GLuint> indices = {0, 1, 2, 0, 2, 3};

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCube(float size) {
    float s = size * 0.5f;

    std::vector<Vertex> vertices = {
        {{-s, -s, -s}, { 0, 0,-1}, {0,0}}, {{ s, -s, -s}, { 0, 0,-1}, {1,0}},
        {{ s,  s, -s}, { 0, 0,-1}, {1,1}}, {{-s,  s, -s}, { 0, 0,-1}, {0,1}},
        {{-s, -s,  s}, { 0, 0, 1}, {1,0}}, {{ s, -s,  s}, { 0, 0, 1}, {0,0}},
        {{ s,  s,  s}, { 0, 0, 1}, {0,1}}, {{-s,  s,  s}, { 0, 0, 1}, {1,1}},
        {{-s,  s, -s}, {-1, 0, 0}, {0,1}}, {{-s, -s, -s}, {-1, 0, 0}, {0,0}},
        {{-s, -s,  s}, {-1, 0, 0}, {1,0}}, {{-s,  s,  s}, {-1, 0, 0}, {1,1}},
        {{ s,  s, -s}, { 1, 0, 0}, {1,1}}, {{ s, -s, -s}, { 1, 0, 0}, {1,0}},
        {{ s, -s,  s}, { 1, 0, 0}, {0,0}}, {{ s,  s,  s}, { 1, 0, 0}, {0,1}},
        {{-s, -s, -s}, { 0,-1, 0}, {0,1}}, {{ s, -s, -s}, { 0,-1, 0}, {1,1}},
        {{ s, -s,  s}, { 0,-1, 0}, {1,0}}, {{-s, -s,  s}, { 0,-1, 0}, {0,0}},
        {{-s,  s, -s}, { 0, 1, 0}, {0,0}}, {{ s,  s, -s}, { 0, 1, 0}, {1,0}},
        {{ s,  s,  s}, { 0, 1, 0}, {1,1}}, {{-s,  s,  s}, { 0, 1, 0}, {0,1}},
    };

    std::vector<GLuint> indices = {
        0,2,1, 0,3,2,   4,5,6, 4,6,7,
        8,9,10, 8,10,11, 12,14,13, 12,15,14,
        16,17,18, 16,18,19, 20,22,21, 20,23,22
    };

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateSphere(float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    for (int y = 0; y <= segments; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSeg = static_cast<float>(x) / static_cast<float>(segments);
            float ySeg = static_cast<float>(y) / static_cast<float>(segments);

            float xPos = std::cos(xSeg * 2.0f * M_PI) * std::sin(ySeg * M_PI) * radius;
            float yPos = std::cos(ySeg * M_PI) * radius;
            float zPos = std::sin(xSeg * 2.0f * M_PI) * std::sin(ySeg * M_PI) * radius;

            glm::vec3 normal = glm::normalize(glm::vec3(xPos, yPos, zPos));
            vertices.emplace_back(glm::vec3(xPos, yPos, zPos), normal,
                                   glm::vec2(xSeg, ySeg));
        }
    }

    for (int y = 0; y < segments; y++) {
        for (int x = 0; x < segments; x++) {
            GLuint a = y * (segments + 1) + x;
            GLuint b = a + 1;
            GLuint c = a + segments + 1;
            GLuint d = c + 1;

            indices.insert(indices.end(), {a, b, d});
            indices.insert(indices.end(), {a, d, c});
        }
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateBox(float w, float h, float d) {
    float hw = w * 0.5f;
    float hh = h * 0.5f;
    float hd = d * 0.5f;

    // UVs 0→1 per face (one image per face, fills the whole face)
    std::vector<Vertex> vertices = {
        // -Z (back, normal 0,0,-1)
        {{-hw, -hh, -hd}, { 0, 0,-1}, {0, 0}},
        {{ hw, -hh, -hd}, { 0, 0,-1}, {1, 0}},
        {{ hw,  hh, -hd}, { 0, 0,-1}, {1, 1}},
        {{-hw,  hh, -hd}, { 0, 0,-1}, {0, 1}},
        // +Z (front, normal 0,0,1) — mirrored U
        {{-hw, -hh,  hd}, { 0, 0, 1}, {1, 0}},
        {{ hw, -hh,  hd}, { 0, 0, 1}, {0, 0}},
        {{ hw,  hh,  hd}, { 0, 0, 1}, {0, 1}},
        {{-hw,  hh,  hd}, { 0, 0, 1}, {1, 1}},
        // -X (left, normal -1,0,0)
        {{-hw,  hh, -hd}, {-1, 0, 0}, {0, 1}},
        {{-hw, -hh, -hd}, {-1, 0, 0}, {0, 0}},
        {{-hw, -hh,  hd}, {-1, 0, 0}, {1, 0}},
        {{-hw,  hh,  hd}, {-1, 0, 0}, {1, 1}},
        // +X (right, normal 1,0,0) — mirrored U
        {{ hw,  hh, -hd}, { 1, 0, 0}, {1, 1}},
        {{ hw, -hh, -hd}, { 1, 0, 0}, {1, 0}},
        {{ hw, -hh,  hd}, { 1, 0, 0}, {0, 0}},
        {{ hw,  hh,  hd}, { 1, 0, 0}, {0, 1}},
        // -Y (bottom, normal 0,-1,0) — mirrored V
        {{-hw, -hh, -hd}, { 0,-1, 0}, {0, 1}},
        {{ hw, -hh, -hd}, { 0,-1, 0}, {1, 1}},
        {{ hw, -hh,  hd}, { 0,-1, 0}, {1, 0}},
        {{-hw, -hh,  hd}, { 0,-1, 0}, {0, 0}},
        // +Y (top, normal 0,1,0) — mirrored U
        {{-hw,  hh, -hd}, { 0, 1, 0}, {1, 0}},
        {{ hw,  hh, -hd}, { 0, 1, 0}, {0, 0}},
        {{ hw,  hh,  hd}, { 0, 1, 0}, {0, 1}},
        {{-hw,  hh,  hd}, { 0, 1, 0}, {1, 1}},
    };

    std::vector<GLuint> indices = {
        0,2,1, 0,3,2,   4,5,6, 4,6,7,
        8,9,10, 8,10,11, 12,14,13, 12,15,14,
        16,17,18, 16,18,19, 20,22,21, 20,23,22
    };

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateWallQuad(float width, float height) {
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float eps = 0.005f;

    // Double-sided vertical quad (XY plane, facing ±Z)
    // Winding matches proven cube pattern:
    //   back (-Z): 0,2,1, 0,3,2   → CCW from -Z (visible from -Z side)
    //   front (+Z): 4,5,6, 4,6,7  → CCW from +Z (visible from +Z side)
    // U=0 at left, U=1 at right; V=0 at bottom, V=1 at top (stb flips V)
    // Both faces use SAME UV orientation when viewed from outside
    std::vector<Vertex> vertices = {
        {{-hw, -hh, -eps}, {0,0,-1}, {0, 0}},
        {{ hw, -hh, -eps}, {0,0,-1}, {1, 0}},
        {{ hw,  hh, -eps}, {0,0,-1}, {1, 1}},
        {{-hw,  hh, -eps}, {0,0,-1}, {0, 1}},
        {{-hw, -hh,  eps}, {0,0, 1}, {0, 0}},
        {{ hw, -hh,  eps}, {0,0, 1}, {1, 0}},
        {{ hw,  hh,  eps}, {0,0, 1}, {1, 1}},
        {{-hw,  hh,  eps}, {0,0, 1}, {0, 1}},
    };

    std::vector<GLuint> indices = {
        0,2,1, 0,3,2,  // back (-Z)
        4,5,6, 4,6,7,  // front (+Z)
    };

    return Mesh(vertices, indices);
}

Mesh Mesh::CreatePlane(float width, float depth) {
    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    std::vector<Vertex> vertices = {
        {{-hw, 0, -hd}, {0,1,0}, {0,0}},
        {{ hw, 0, -hd}, {0,1,0}, {1,0}},
        {{ hw, 0,  hd}, {0,1,0}, {1,1}},
        {{-hw, 0,  hd}, {0,1,0}, {0,1}},
    };

    std::vector<GLuint> indices = {0, 1, 2, 0, 2, 3};

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCylinder(float radius, float height, int segments) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    float hh = height * 0.5f;

    // Top center
    vertices.emplace_back(glm::vec3(0, hh, 0), glm::vec3(0, 1, 0), glm::vec2(0.5f, 0.5f));
    // Bottom center
    vertices.emplace_back(glm::vec3(0, -hh, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f));

    for (int i = 0; i <= segments; i++) {
        float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * M_PI;
        float c = std::cos(angle);
        float s = std::sin(angle);
        float u = static_cast<float>(i) / static_cast<float>(segments);

        // Top ring
        vertices.emplace_back(glm::vec3(c * radius, hh, s * radius), glm::vec3(0, 1, 0), glm::vec2(c * 0.5f + 0.5f, s * 0.5f + 0.5f));
        // Bottom ring
        vertices.emplace_back(glm::vec3(c * radius, -hh, s * radius), glm::vec3(0, -1, 0), glm::vec2(c * 0.5f + 0.5f, s * 0.5f + 0.5f));
        // Side top
        vertices.emplace_back(glm::vec3(c * radius, hh, s * radius), glm::vec3(c, 0, s), glm::vec2(u, 0));
        // Side bottom
        vertices.emplace_back(glm::vec3(c * radius, -hh, s * radius), glm::vec3(c, 0, s), glm::vec2(u, 1));
    }

    int topCenter = 0, bottomCenter = 1;
    for (int i = 0; i < segments; i++) {
        int t0 = 2 + i * 4;
        int t1 = 2 + ((i + 1) % segments) * 4;
        int b0 = 3 + i * 4;
        int b1 = 3 + ((i + 1) % segments) * 4;
        int st0 = 4 + i * 4;
        int st1 = 4 + ((i + 1) % segments) * 4;
        int sb0 = 5 + i * 4;
        int sb1 = 5 + ((i + 1) % segments) * 4;

        // Top cap
        indices.insert(indices.end(), {static_cast<GLuint>(topCenter), static_cast<GLuint>(t1), static_cast<GLuint>(t0)});
        // Bottom cap
        indices.insert(indices.end(), {static_cast<GLuint>(bottomCenter), static_cast<GLuint>(b0), static_cast<GLuint>(b1)});
        // Side
        indices.insert(indices.end(), {static_cast<GLuint>(st0), static_cast<GLuint>(sb1), static_cast<GLuint>(sb0), static_cast<GLuint>(st0), static_cast<GLuint>(st1), static_cast<GLuint>(sb1)});
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCone(float radius, float height, int segments) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    float hh = height * 0.5f;

    // Apex
    vertices.emplace_back(glm::vec3(0, hh, 0), glm::normalize(glm::vec3(0, 1, 0)), glm::vec2(0.5f, 0.5f));
    // Bottom center
    vertices.emplace_back(glm::vec3(0, -hh, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f));

    for (int i = 0; i <= segments; i++) {
        float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * M_PI;
        float c = std::cos(angle);
        float s = std::sin(angle);
        float u = static_cast<float>(i) / static_cast<float>(segments);

        glm::vec3 sideNorm = glm::normalize(glm::vec3(c, radius / height, s));
        // Bottom ring
        vertices.emplace_back(glm::vec3(c * radius, -hh, s * radius), glm::vec3(0, -1, 0), glm::vec2(c * 0.5f + 0.5f, s * 0.5f + 0.5f));
        // Side bottom
        vertices.emplace_back(glm::vec3(c * radius, -hh, s * radius), sideNorm, glm::vec2(u, 1));
    }

    int apex = 0, bottomCenter = 1;
    for (int i = 0; i < segments; i++) {
        int b0 = 2 + i * 2;
        int b1 = 2 + ((i + 1) % segments) * 2;
        int sb0 = 3 + i * 2;
        int sb1 = 3 + ((i + 1) % segments) * 2;

        // Bottom cap
        indices.insert(indices.end(), {static_cast<GLuint>(bottomCenter), static_cast<GLuint>(b0), static_cast<GLuint>(b1)});
        // Side
        indices.insert(indices.end(), {static_cast<GLuint>(apex), static_cast<GLuint>(sb0), static_cast<GLuint>(sb1)});
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateTorus(float radius, float tubeRadius, int segments, int tubeSegments) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    for (int i = 0; i <= segments; i++) {
        float u = static_cast<float>(i) / static_cast<float>(segments);
        float outerAngle = u * 2.0f * M_PI;

        for (int j = 0; j <= tubeSegments; j++) {
            float v = static_cast<float>(j) / static_cast<float>(tubeSegments);
            float innerAngle = v * 2.0f * M_PI;

            float x = (radius + tubeRadius * std::cos(innerAngle)) * std::cos(outerAngle);
            float y = tubeRadius * std::sin(innerAngle);
            float z = (radius + tubeRadius * std::cos(innerAngle)) * std::sin(outerAngle);

            glm::vec3 norm = glm::normalize(glm::vec3(
                std::cos(innerAngle) * std::cos(outerAngle),
                std::sin(innerAngle),
                std::cos(innerAngle) * std::sin(outerAngle)));

            vertices.emplace_back(glm::vec3(x, y, z), norm, glm::vec2(u, v));
        }
    }

    for (int i = 0; i < segments; i++) {
        for (int j = 0; j < tubeSegments; j++) {
            int a = i * (tubeSegments + 1) + j;
            int b = a + 1;
            int c = (i + 1) * (tubeSegments + 1) + j;
            int d = c + 1;
            indices.insert(indices.end(), {static_cast<GLuint>(a), static_cast<GLuint>(c), static_cast<GLuint>(b), static_cast<GLuint>(b), static_cast<GLuint>(c), static_cast<GLuint>(d)});
        }
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateIcosahedron(float radius) {
    float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

    std::vector<glm::vec3> pts = {
        {-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
        { 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
        { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1},
    };

    for (auto& p : pts) p = glm::normalize(p) * radius;

    std::vector<GLuint> idx = {
        0,11,5, 0,5,1, 0,1,7, 0,7,10, 0,10,11,
        1,5,9, 5,11,4, 11,10,2, 10,7,6, 7,1,8,
        3,9,4, 3,4,2, 3,2,6, 3,6,8, 3,8,9,
        4,9,5, 2,4,11, 6,2,10, 8,6,7, 9,8,1
    };

    std::vector<Vertex> vertices;
    for (auto& i : idx) {
        glm::vec3 n = glm::normalize(pts[i]);
        vertices.emplace_back(pts[i], n, glm::vec2(0, 0));
    }

    std::vector<GLuint> indices;
    for (GLuint i = 0; i < vertices.size(); i++) {
        indices.push_back(i);
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateGrid(float width, float depth, int divisions) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    for (int z = 0; z <= divisions; z++) {
        for (int x = 0; x <= divisions; x++) {
            float u = static_cast<float>(x) / static_cast<float>(divisions);
            float v = static_cast<float>(z) / static_cast<float>(divisions);
            float px = -hw + u * width;
            float pz = -hd + v * depth;
            vertices.emplace_back(glm::vec3(px, 0, pz), glm::vec3(0, 1, 0), glm::vec2(u, v));
        }
    }

    for (int z = 0; z < divisions; z++) {
        for (int x = 0; x < divisions; x++) {
            int a = z * (divisions + 1) + x;
            int b = a + 1;
            int c = a + (divisions + 1);
            int d = c + 1;
            indices.insert(indices.end(), {static_cast<GLuint>(a), static_cast<GLuint>(c), static_cast<GLuint>(b), static_cast<GLuint>(b), static_cast<GLuint>(c), static_cast<GLuint>(d)});
        }
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCircle(float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec2(0.5f, 0.5f));

    for (int i = 0; i <= segments; i++) {
        float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * M_PI;
        float c = std::cos(angle);
        float s = std::sin(angle);
        vertices.emplace_back(glm::vec3(c * radius, 0, s * radius), glm::vec3(0, 1, 0),
                              glm::vec2(c * 0.5f + 0.5f, s * 0.5f + 0.5f));
    }

    for (int i = 1; i <= segments; i++) {
        indices.insert(indices.end(), {0, static_cast<GLuint>(i), static_cast<GLuint>(i + 1)});
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateTetrahedron(float radius) {
    float a = radius / std::sqrt(3.0f);
    std::vector<glm::vec3> pts = {
        { a,  a,  a}, { a, -a, -a}, {-a,  a, -a}, {-a, -a,  a}
    };

    std::vector<GLuint> idx = {0,1,2, 0,3,1, 0,2,3, 1,3,2};

    std::vector<Vertex> vertices;
    for (size_t i = 0; i < idx.size(); i++) {
        glm::vec3 n = glm::normalize(glm::cross(
            pts[idx[(i/3)*3 + 1]] - pts[idx[(i/3)*3]],
            pts[idx[(i/3)*3 + 2]] - pts[idx[(i/3)*3]]));
        vertices.emplace_back(pts[idx[i]], n, glm::vec2(0, 0));
    }

    std::vector<GLuint> indices;
    for (GLuint i = 0; i < vertices.size(); i++) indices.push_back(i);

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateOctahedron(float radius) {
    std::vector<glm::vec3> pts = {
        { 0,  radius, 0}, { radius, 0, 0}, { 0, 0,  radius},
        {-radius, 0, 0}, { 0, 0, -radius}, { 0, -radius, 0}
    };

    std::vector<GLuint> idx = {
        0,1,2, 0,2,3, 0,3,4, 0,4,1,
        5,2,1, 5,3,2, 5,4,3, 5,1,4
    };

    std::vector<Vertex> vertices;
    for (size_t i = 0; i < idx.size(); i++) {
        glm::vec3 n = glm::normalize(glm::cross(
            pts[idx[(i/3)*3 + 1]] - pts[idx[(i/3)*3]],
            pts[idx[(i/3)*3 + 2]] - pts[idx[(i/3)*3]]));
        vertices.emplace_back(pts[idx[i]], n, glm::vec2(0, 0));
    }

    std::vector<GLuint> indices;
    for (GLuint i = 0; i < vertices.size(); i++) indices.push_back(i);

    return Mesh(vertices, indices);
}

} // namespace planet
