#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <ncurses.h>

struct lua_State;
struct GLFWwindow;

namespace planet {

struct EnvEntry {
    std::string name;
    std::string type;       // "light" / "mesh"
    std::string lightType;  // "directional" / "point" / "spot"
    glm::vec3 lightColor{1.0f};
    float lightRange = 10.0f;
    std::string meshType;
    glm::vec4 meshColor{1.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    std::string texture;
};

struct TplEntry {
    std::string name;
    std::string meshType;
    glm::vec4 color{1.0f};
    glm::vec3 scale{1.0f};
    std::string texture;
    bool isLight = false;
    std::string lightType;
    glm::vec3 lightColor{1.0f};
    float lightRange = 10.0f;
};

class Mapper {
public:
    static Mapper& Instance();

    void Init(int argc, char** argv);
    void Run();
    void Shutdown();

private:
    Mapper() = default;
    Mapper(const Mapper&) = delete;
    Mapper& operator=(const Mapper&) = delete;

    enum Mode { MODE_ENV, MODE_TPL };

    void DrawAll();
    void DrawHeader();
    void DrawList();
    void DrawDetail();
    void DrawStatus();
    void HandleInput(int ch);

    void LoadFromLua(const std::string& path);
    bool SaveToLua();
    void ParseEnvTable(lua_State* L, int idx);
    void ParseTplTable(lua_State* L, int idx);

    void AddEnv();
    void AddTpl();
    void DeleteCurrent();
    void EditField(const std::string& prompt, std::string& val);
    void EditFloat3(const std::string& prompt, glm::vec3& v);
    void EditFloat4(const std::string& prompt, glm::vec4& v);
    void InputBox(const std::string& prompt, std::string& out);

    void ToggleViewport();
    void RenderViewport();
    void PopulateScene();

    std::string m_path;
    std::vector<EnvEntry> m_env;
    std::vector<TplEntry> m_tpl;
    Mode m_mode = MODE_ENV;
    int m_sel = 0;
    int m_scroll = 0;
    int m_detailScroll = 0;
    bool m_dirty = false;

    int m_termH = 0, m_termW = 0;

    GLFWwindow* m_window = nullptr;
    bool m_vpVis = false;
    std::unique_ptr<class Framebuffer> m_fbo;
    std::unique_ptr<class Camera> m_cam;
    int m_vpW = 0, m_vpH = 0;

    enum { CP_NORMAL=1, CP_SEL, CP_HDR, CP_STAT, CP_ACT, CP_TITLE, CP_INPUT };
};

} // namespace planet
