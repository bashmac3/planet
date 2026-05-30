#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <ncurses.h>

struct GLFWwindow;

namespace planet {

class Entity;
class Mesh;
class Camera;
class Framebuffer;

class Editor {
public:
    static Editor& Instance();

    void Init(GLFWwindow* window);
    void Shutdown();
    void Run();

    void SelectEntity(Entity* entity);
    void Deselect();
    Entity* GetSelectedEntity() const { return m_selected; }

    int GetEntityCount() const;
    Entity* GetEntityByIndex(int index) const;

    void SetGridSnap(float snap) { m_gridSnap = snap; }
    float GetGridSnap() const { return m_gridSnap; }
    void SetShowGrid(bool show) { m_showGrid = show; }

private:
    Editor() = default;
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    enum class ContentTab { Script, Info, Project };

    // Layout
    void RecalcLayout();
    int  LH(int line) const; // header y
    int  LL(int line) const; // left panel y
    int  LC(int line) const; // center y
    int  LR(int line) const; // right y
    int  LB(int line) const; // bottom y

    // Drawing
    void DrawAll();
    void DrawHeader();
    void DrawSceneList();
    void DrawContentArea();
    void DrawScriptContent();
    void DrawInfoContent();
    void DrawProjectContent();
    void DrawProperties();
    void DrawLog();
    void DrawStatusBar();

    // Input
    void HandleInput(int ch);
    void HandleGlobalKeys(int ch);
    void HandleSceneListKeys(int ch);
    void HandleContentKeys(int ch);
    void HandlePropsKeys(int ch);
    void HandleLogKeys(int ch);
    void HandleScriptKeys(int ch);
    void HandleMouseClick(MEVENT& ev);

    // Entity operations
    void DeleteSelected();
    void DuplicateSelected();
    void CreateEntity(const std::string& name, int type); // 0=cube,1=sphere,2=light

    // Script helpers
    std::vector<std::string> GetScriptLines();
    void ScriptPutChar(char c);
    void ScriptBackspace();
    void ScriptNewline();
    void ScriptSave();

    // Viewport rendering (optional GLFW window)
    void ToggleViewport();
    void RenderViewport();

    // State
    GLFWwindow* m_window = nullptr;
    bool m_running = true;
    Entity* m_selected = nullptr;
    ContentTab m_contentTab = ContentTab::Info;
    int m_sceneScroll = 0;
    int m_logScroll = 0;
    int m_propScroll = 0;
    int m_scriptScrollY = 0;
    int m_scriptScrollX = 0;
    int m_scriptCursorY = 0;
    int m_scriptCursorX = 0;

    // Focus: 0=scene, 1=content, 2=props, 3=log
    int m_focus = 0;

    // Terminal dimensions at last draw
    int m_termH = 0, m_termW = 0;
    int m_leftW = 0, m_rightW = 0, m_logH = 0;

    // Transform state (for entity manipulation)
    enum class TransformOp { None, Translate, Rotate, Scale };
    TransformOp m_transformOp = TransformOp::None;
    int m_constrainAxis = -1;
    glm::vec3 m_transformStart{0.0f};

    // Panel visibility
    bool m_showGrid = true;
    float m_gridSnap = 0.5f;

    // Viewport window
    bool m_viewportVisible = false;
    std::unique_ptr<Framebuffer> m_viewportFbo;
    std::unique_ptr<Camera> m_editorCamera;
    int m_vpWidth = 0, m_vpHeight = 0;

    // Entity meshes for creation
    std::unique_ptr<Mesh> m_entityCube;
    std::unique_ptr<Mesh> m_entitySphere;

    // ncurses color pairs
    enum { CP_NORMAL = 1, CP_SELECTED, CP_HIGHLIGHT, CP_HEADER,
           CP_STATUS, CP_ERROR, CP_WARN, CP_INFO, CP_TITLE, CP_ACTIVE };
};

} // namespace planet
