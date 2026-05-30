#include "planet/editor/editor.h"
#include "planet/editor/project.h"
#include "planet/editor/script_editor.h"
#include "planet/editor/log_console.h"
#include "planet/core/scene.h"
#include "planet/core/camera.h"
#include "planet/core/input.h"
#include "planet/render/renderer.h"
#include "planet/render/model_renderer.h"
#include "planet/render/mesh.h"
#include "planet/render/framebuffer.h"
#include "planet/render/texture.h"
#include "planet/physics/physics.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/system.h"
#include "planet/ecs/component.h"
#include "planet/resource/resource_manager.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <clocale>
#include <chrono>
#include <thread>

namespace planet {

// ── Singleton ──────────────────────────────────────────────────

Editor& Editor::Instance() {
    static Editor instance;
    return instance;
}

// ── Init / Shutdown ────────────────────────────────────────────

void Editor::Init(GLFWwindow* window) {
    m_window = window;

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(16); // ms — enough for escape sequences to complete, short enough for responsive UI
    curs_set(0);
    start_color();

    // Enable mouse tracking
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0); // no click debounce delay
    printf("\033[?1002h\033[?1006h"); // enable button-event + SGR extended mouse tracking

    init_pair(CP_NORMAL,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(CP_SELECTED, COLOR_CYAN,    COLOR_BLACK);
    init_pair(CP_HIGHLIGHT,COLOR_BLACK,   COLOR_CYAN);
    init_pair(CP_HEADER,   COLOR_WHITE,   COLOR_BLUE);
    init_pair(CP_STATUS,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(CP_ERROR,    COLOR_RED,     COLOR_BLACK);
    init_pair(CP_WARN,     COLOR_YELLOW,  COLOR_BLACK);
    init_pair(CP_INFO,     COLOR_GREEN,   COLOR_BLACK);
    init_pair(CP_TITLE,    COLOR_YELLOW,  COLOR_BLUE);
    init_pair(CP_ACTIVE,   COLOR_BLACK,   COLOR_CYAN);

    // Entity meshes
    m_entityCube = std::make_unique<Mesh>(Mesh::CreateCube(1.0f));
    m_entitySphere = std::make_unique<Mesh>(Mesh::CreateSphere(0.5f, 16));

    // Viewport framebuffer + camera
    m_viewportFbo = std::make_unique<Framebuffer>();
    m_viewportFbo->Init(128, 128);
    m_editorCamera = std::make_unique<Camera>();
    m_editorCamera->SetFarPlane(1000.0f);
}

void Editor::Shutdown() {
    printf("\033[?1002l\033[?1006l"); // disable mouse tracking
    m_viewportFbo.reset();
    m_editorCamera.reset();
    m_entityCube.reset();
    m_entitySphere.reset();
    curs_set(1);
    endwin();
}

// ── Layout calculation ─────────────────────────────────────────

void Editor::RecalcLayout() {
    getmaxyx(stdscr, m_termH, m_termW);
    m_leftW   = std::max(20, m_termW * 25 / 100);
    m_rightW  = std::max(22, m_termW * 25 / 100);
    m_logH    = std::max(4,  m_termH / 5);
}

inline int Editor::LH(int line) const { return line; }
inline int Editor::LL(int line) const { return 2 + line; }
inline int Editor::LC(int line) const { return 2 + line; }
inline int Editor::LR(int line) const { return 2 + line; }
inline int Editor::LB(int line) const { return m_termH - m_logH - 1 + line; }

// ── Main run loop ──────────────────────────────────────────────

void Editor::Run() {
    m_running = true;
    RecalcLayout();

    int lastW = 0, lastH = 0;

    while (m_running) {
        // Terminal resize handling
        getmaxyx(stdscr, m_termH, m_termW);
        if (m_termW != lastW || m_termH != lastH) {
            RecalcLayout();
            lastW = m_termW; lastH = m_termH;
        }

        // Draw
        DrawAll();
        doupdate();

        // Viewport rendering
        if (m_viewportVisible) {
            RenderViewport();
        }

        // Poll GLFW events (for viewport window)
        glfwPollEvents();

        // Input
        int ch = getch();
        if (ch != ERR) {
            HandleInput(ch);
        }
    }
}

// ── Input ──────────────────────────────────────────────────────

void Editor::HandleInput(int ch) {
    // Mouse
    if (ch == KEY_MOUSE) {
        MEVENT ev;
        if (getmouse(&ev) == OK) {
            HandleMouseClick(ev);
        }
        return;
    }

    // Global keys
    if (ch == '\t') {
        m_focus = (m_focus + 1) % 4;
        return;
    }
    if (ch == 27) { // ESC
        Deselect(); m_transformOp = TransformOp::None;
        return;
    }
    if (ch == 'q' || ch == 'Q') { m_running = false; return; }

    // F-keys: match on the actual KEY_F(n) codes
    if (ch >= KEY_F(1) && ch <= KEY_F(12)) {
        switch (ch) {
            case KEY_F(1): m_contentTab = ContentTab::Info;    m_focus = 1; return;
            case KEY_F(2): m_contentTab = ContentTab::Script;  m_focus = 1; return;
            case KEY_F(3): m_contentTab = ContentTab::Project; m_focus = 1; return;
            case KEY_F(4): m_focus = 0; return;
            case KEY_F(5): EditorLog("Build triggered", 0); return;
            case KEY_F(6): EditorLog("Deploy triggered", 0); return;
            case KEY_F(8): ToggleViewport(); return;
            case KEY_F(10): m_running = false; return;
        }
        return;
    }

    // Route to focused panel
    switch (m_focus) {
        case 0: HandleSceneListKeys(ch); break;
        case 1: HandleContentKeys(ch);   break;
        case 2: HandlePropsKeys(ch);     break;
        case 3: HandleLogKeys(ch);       break;
    }
}

// ── Mouse handling ────────────────────────────────────────────

void Editor::HandleMouseClick(MEVENT& ev) {
    int y = ev.y, x = ev.x;

    // Header (line 0) — click tabs
    if (y == 0) {
        // "F1:Info  F2:Script  F3:Project" starts at x=2
        // Each is ~9 chars + padding
        if (x >= 2 && x < 9) { m_contentTab = ContentTab::Info;    m_focus = 1; return; }
        if (x >= 12 && x < 21) { m_contentTab = ContentTab::Script;  m_focus = 1; return; }
        if (x >= 24 && x < 34) { m_contentTab = ContentTab::Project; m_focus = 1; return; }
        return;
    }

    // Status bar (line 1) — no interactive elements currently
    if (y == 1) return;

    // Scene outliner (left panel)
    int sceneTop = 2;
    int sceneBot = m_termH - m_logH - 3;
    if (x >= 0 && x < m_leftW && y >= sceneTop && y < sceneBot) {
        int idx = (y - sceneTop) + m_sceneScroll;
        auto& scene = Scene::Instance();
        if (idx >= 0 && idx < (int)scene.GetEntityCount()) {
            m_focus = 0;
            m_sceneScroll = std::max(0, idx - 0); // ensure visible
            SelectEntity(scene.GetEntityAtIndex(idx));
        }
        return;
    }

    // Content area (center)
    int contentLeft = m_leftW + 1;
    int contentRight = m_termW - m_rightW - 1;
    if (x >= contentLeft && x < contentRight && y >= sceneTop && y < sceneBot) {
        m_focus = 1;
        return;
    }

    // Properties panel (right)
    int propsLeft = m_termW - m_rightW;
    if (x >= propsLeft && x < m_termW && y >= sceneTop && y < sceneBot) {
        m_focus = 2;
        return;
    }

    // Log console (bottom)
    int logTop = m_termH - m_logH - 1;
    if (y >= logTop && y < m_termH) {
        m_focus = 3;
        return;
    }
}

// ── Global key shortcuts ───────────────────────────────────────

void Editor::HandleSceneListKeys(int ch) {
    auto& scene = Scene::Instance();
    switch (ch) {
        case 'j': case KEY_DOWN:
            m_sceneScroll++;
            if (m_sceneScroll >= (int)scene.GetEntityCount())
                m_sceneScroll = (int)scene.GetEntityCount() - 1;
            break;
        case 'k': case KEY_UP:
            m_sceneScroll = std::max(0, m_sceneScroll - 1);
            break;
        case '\n':
            if (m_sceneScroll >= 0 && m_sceneScroll < (int)scene.GetEntityCount()) {
                SelectEntity(scene.GetEntityAtIndex(m_sceneScroll));
                m_focus = 2; // jump to properties
            }
            break;
        case 'c': CreateEntity("Cube", 0);   break;
        case 's': CreateEntity("Sphere", 1); break;
        case 'l': CreateEntity("Light", 2);  break;
        case 'd': DeleteSelected();          break;
        case 'D': DuplicateSelected();       break;
        case 'g': if (m_selected) { m_transformOp = TransformOp::Translate; m_transformStart = m_selected->GetPosition(); } break;
        case 'r': if (m_selected) { m_transformOp = TransformOp::Rotate;    m_transformStart = m_selected->GetEulerAngles(); } break;
        case 't': if (m_selected) { m_transformOp = TransformOp::Scale;     m_transformStart = m_selected->GetScale(); } break;
        case 'G':
            m_sceneScroll = (int)scene.GetEntityCount() - 1;
            if (m_sceneScroll < 0) m_sceneScroll = 0;
            break;
    }
}

void Editor::HandleContentKeys(int ch) {
    if (m_contentTab == ContentTab::Script && ScriptEditor::Instance().IsOpen()) {
        HandleScriptKeys(ch);
    }
}

void Editor::HandlePropsKeys(int ch) {
    if (ch == 'j' || ch == KEY_DOWN)  m_propScroll++;
    if (ch == 'k' || ch == KEY_UP)    m_propScroll = std::max(0, m_propScroll - 1);
}

void Editor::HandleLogKeys(int ch) {
    if (ch == 'j' || ch == KEY_DOWN)  m_logScroll++;
    if (ch == 'k' || ch == KEY_UP)    m_logScroll = std::max(0, m_logScroll - 1);
}

// ── Script editing ─────────────────────────────────────────────

std::vector<std::string> Editor::GetScriptLines() {
    auto* tab = ScriptEditor::Instance().GetActiveTab();
    if (!tab) return {};
    std::vector<std::string> lines;
    std::string s;
    for (char c : tab->content) {
        if (c == '\n') { lines.push_back(s); s.clear(); }
        else s += c;
    }
    lines.push_back(s);
    return lines;
}

void Editor::ScriptPutChar(char c) {
    auto* tab = ScriptEditor::Instance().GetActiveTab();
    if (!tab) return;
    auto lines = GetScriptLines();
    int idx = 0;
    for (int i = 0; i < m_scriptCursorY && i < (int)lines.size(); i++)
        idx += lines[i].size() + 1;
    idx += m_scriptCursorX;
    if (idx <= (int)tab->content.size()) {
        tab->content.insert(idx, 1, c);
        tab->dirty = true;
        m_scriptCursorX++;
    }
}

void Editor::ScriptBackspace() {
    auto* tab = ScriptEditor::Instance().GetActiveTab();
    if (!tab || tab->content.empty()) return;
    auto lines = GetScriptLines();
    int idx = 0;
    for (int i = 0; i < m_scriptCursorY && i < (int)lines.size(); i++)
        idx += lines[i].size() + 1;
    idx += m_scriptCursorX;
    if (idx > 0) {
        tab->content.erase(idx - 1, 1);
        tab->dirty = true;
        if (m_scriptCursorX > 0) m_scriptCursorX--;
        else if (m_scriptCursorY > 0) {
            m_scriptCursorY--;
            auto lines2 = GetScriptLines();
            if (m_scriptCursorY < (int)lines2.size())
                m_scriptCursorX = (int)lines2[m_scriptCursorY].size();
        }
    }
}

void Editor::ScriptNewline() {
    auto* tab = ScriptEditor::Instance().GetActiveTab();
    if (!tab) return;
    auto lines = GetScriptLines();
    int idx = 0;
    for (int i = 0; i < m_scriptCursorY && i < (int)lines.size(); i++)
        idx += lines[i].size() + 1;
    idx += m_scriptCursorX;
    tab->content.insert(idx, 1, '\n');
    tab->dirty = true;
    m_scriptCursorY++;
    m_scriptCursorX = 0;
}

void Editor::ScriptSave() {
    auto* tab = ScriptEditor::Instance().GetActiveTab();
    if (tab) {
        ScriptEditor::Instance().SaveCurrentFile();
        EditorLog("Script saved", 0);
    }
}

void Editor::HandleScriptKeys(int ch) {
    if (ch >= 32 && ch <= 126) {
        ScriptPutChar((char)ch);
        return;
    }
    switch (ch) {
        case '\n': case KEY_ENTER: ScriptNewline(); break;
        case KEY_BACKSPACE: case 127: ScriptBackspace(); break;
        case KEY_LEFT:
            if (m_scriptCursorX > 0) m_scriptCursorX--;
            else if (m_scriptCursorY > 0) {
                m_scriptCursorY--;
                auto lines = GetScriptLines();
                if (m_scriptCursorY < (int)lines.size())
                    m_scriptCursorX = (int)lines[m_scriptCursorY].size();
            }
            break;
        case KEY_RIGHT: {
            auto lines = GetScriptLines();
            if (m_scriptCursorY < (int)lines.size() &&
                m_scriptCursorX < (int)lines[m_scriptCursorY].size())
                m_scriptCursorX++;
            else if (m_scriptCursorY < (int)lines.size() - 1) {
                m_scriptCursorY++;
                m_scriptCursorX = 0;
            }
            break;
        }
        case KEY_UP:
            if (m_scriptCursorY > 0) m_scriptCursorY--;
            break;
        case KEY_DOWN: {
            auto lines = GetScriptLines();
            if (m_scriptCursorY < (int)lines.size() - 1) m_scriptCursorY++;
            break;
        }
        case KEY_HOME: m_scriptCursorX = 0; break;
        case KEY_END: {
            auto lines = GetScriptLines();
            if (m_scriptCursorY < (int)lines.size())
                m_scriptCursorX = (int)lines[m_scriptCursorY].size();
            break;
        }
        case KEY_PPAGE: m_scriptScrollY = std::max(0, m_scriptScrollY - 10); break;
        case KEY_NPAGE: m_scriptScrollY += 10; break;
        case 18: // Ctrl+R
            ScriptEditor::Instance().OpenFile("/tmp/new.lua"); break;
        case 19: // Ctrl+S
            ScriptSave(); break;
        case 15: // Ctrl+O
            EditorLog("Open script via command not implemented in TUI", 0); break;
    }
}

// ── Entity operations ──────────────────────────────────────────

void Editor::SelectEntity(Entity* entity) {
    m_selected = entity;
    EditorLog("Selected: " + (entity ? entity->GetName() : std::string("none")), 0);
}

void Editor::Deselect() {
    m_selected = nullptr;
    m_transformOp = TransformOp::None;
    m_constrainAxis = -1;
}

void Editor::DeleteSelected() {
    if (!m_selected) return;
    Entity* victim = m_selected;
    m_selected = nullptr;
    m_transformOp = TransformOp::None;
    Scene::Instance().DestroyEntity(victim);
    EditorLog("Entity deleted", 0);
}

void Editor::DuplicateSelected() {
    if (!m_selected) return;
    Entity* clone = Scene::Instance().CreateEntity(m_selected->GetName() + " (copy)");
    if (clone) {
        clone->SetPosition(m_selected->GetPosition() + glm::vec3(1.0f, 0.0f, 0.0f));
        clone->SetRotation(m_selected->GetRotation());
        clone->SetScale(m_selected->GetScale());
        SelectEntity(clone);
        EditorLog("Entity duplicated", 0);
    }
}

void Editor::CreateEntity(const std::string& name, int type) {
    Entity* ent = Scene::Instance().CreateEntity(name);
    if (!ent) return;
    if (type == 0) { // cube
        auto* mc = ent->AddComponent<MeshComponent>();
        mc->mesh = m_entityCube.get();
        mc->color = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
    } else if (type == 1) { // sphere
        auto* mc = ent->AddComponent<MeshComponent>();
        mc->mesh = m_entitySphere.get();
        mc->color = glm::vec4(0.2f, 0.6f, 0.3f, 1.0f);
    } else if (type == 2) { // light
        auto* lc = ent->AddComponent<LightComponent>();
        lc->type = LightComponent::Directional;
        lc->color = glm::vec3(1.0f, 0.95f, 0.85f);
        ent->SetEulerAngles(glm::vec3(45, -30, 0));
    }
    SelectEntity(ent);
    EditorLog("Created: " + name, 0);
}

int Editor::GetEntityCount() const {
    return (int)Scene::Instance().GetEntityCount();
}

Entity* Editor::GetEntityByIndex(int index) const {
    return Scene::Instance().GetEntityAtIndex(index);
}

// ── Viewport (optional GLFW window) ────────────────────────────

void Editor::ToggleViewport() {
    m_viewportVisible = !m_viewportVisible;
    if (m_viewportVisible) {
        glfwShowWindow(m_window);
        EditorLog("Viewport window shown", 0);
    } else {
        glfwHideWindow(m_window);
        EditorLog("Viewport window hidden", 0);
    }
}

void Editor::RenderViewport() {
    if (!m_viewportVisible || !m_window) return;

    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    if (fbW < 1 || fbH < 1) return;

    if (fbW != m_vpWidth || fbH != m_vpHeight) {
        m_viewportFbo->Init(fbW, fbH);
        m_vpWidth = fbW; m_vpHeight = fbH;
    }

    auto& renderer = Renderer::Instance();
    auto& modelRenderer = ModelRenderer::Instance();

    Camera* gameCam = Scene::Instance().GetActiveCamera();
    if (!gameCam) return;

    // Editor camera orbit
    m_editorCamera->SetPosition(glm::vec3(10, 8, 10));
    m_editorCamera->SetTarget(glm::vec3(0, 0, 0));
    m_editorCamera->SetFOV(gameCam->GetFOV());

    m_viewportFbo->Bind();
    glViewport(0, 0, fbW, fbH);
    renderer.ClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer.SetViewMatrix(m_editorCamera->GetViewMatrix());
    renderer.SetProjectionMatrix(m_editorCamera->GetProjectionMatrix((float)fbW / (float)fbH));

    ModelRenderer::Instance().BeginFrame();
    SystemManager::Instance().UpdateAll(0.0);
    ModelRenderer::Instance().EndFrame();

    m_viewportFbo->Unbind();

    // Blit to screen
    glad_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glad_glBindFramebuffer(GL_READ_FRAMEBUFFER, m_viewportFbo->GetFboId());
    glad_glBlitFramebuffer(0, 0, fbW, fbH, 0, 0, fbW, fbH,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glfwSwapBuffers(m_window);
}

// ═══════════════════════════════════════════════════════════════
// TUI DRAWING
// ═══════════════════════════════════════════════════════════════

void Editor::DrawAll() {
    erase();
    DrawHeader();
    DrawSceneList();
    DrawContentArea();
    DrawProperties();
    DrawLog();
    DrawStatusBar();
}

// ── Header ─────────────────────────────────────────────────────

void Editor::DrawHeader() {
    attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
    for (int x = 0; x < m_termW; x++) mvaddch(0, x, ' ');

    const char* tabs[] = {"F1:Info", "F2:Script", "F3:Project"};
    int xOff = 2;
    for (int i = 0; i < 3; i++) {
        int tab = (int)m_contentTab;
        if (i == tab) attron(COLOR_PAIR(CP_TITLE) | A_BOLD);
        else          attron(COLOR_PAIR(CP_HEADER));
        mvprintw(0, xOff, " %s ", tabs[i]);
        xOff += strlen(tabs[i]) + 3;
        attron(COLOR_PAIR(CP_HEADER));
    }
    mvprintw(0, xOff, "  F4:Scene  F5:Build  F6:Deploy  F8:Viewport  F10:Exit  Q:Quit");
    attroff(COLOR_PAIR(CP_HEADER) | A_BOLD);

    // Selection info line
    attron(COLOR_PAIR(CP_STATUS));
    for (int x = 0; x < m_termW; x++) mvaddch(1, x, ' ');
    int col = 1;
    if (m_selected) {
        mvprintw(1, col, "Sel: %s ", m_selected->GetName().c_str()); col += 20;
        auto pos = m_selected->GetPosition();
        mvprintw(1, col, "Pos: %.1f %.1f %.1f ", pos.x, pos.y, pos.z); col += 22;
    } else {
        mvprintw(1, col, "Sel: <none>"); col += 15;
    }
    switch (m_transformOp) {
        case TransformOp::Translate: mvprintw(1, col, "Mode: TRANSLATE"); col += 17; break;
        case TransformOp::Rotate:    mvprintw(1, col, "Mode: ROTATE");    col += 13; break;
        case TransformOp::Scale:     mvprintw(1, col, "Mode: SCALE");    col += 13; break;
        default:                     mvprintw(1, col, "Mode: Object");    col += 13; break;
    }
    mvprintw(1, col, "  Grid: %s Snap: %.1f", m_showGrid ? "ON" : "OFF", m_gridSnap);
    attroff(COLOR_PAIR(CP_STATUS));
}

// ── Scene Outliner ─────────────────────────────────────────────

void Editor::DrawSceneList() {
    int y0 = 2, w = m_leftW, h = m_termH - m_logH - 3;
    int x0 = 0;

    // Border
    attron(COLOR_PAIR(m_focus == 0 ? CP_ACTIVE : CP_NORMAL));
    for (int y = y0; y < y0 + h && y < m_termH; y++) {
        mvaddch(y, x0 + w, ' ');
    }
    mvprintw(y0 - 1, x0 + 1, " Scene Outliner ");
    attroff(COLOR_PAIR(m_focus == 0 ? CP_ACTIVE : CP_NORMAL));

    auto& scene = Scene::Instance();
    int count = (int)scene.GetEntityCount();
    int visLines = h - 1;
    if (m_sceneScroll > count - visLines) m_sceneScroll = std::max(0, count - visLines);

    for (int i = 0; i < visLines && (m_sceneScroll + i) < count; i++) {
        Entity* ent = scene.GetEntityAtIndex(m_sceneScroll + i);
        if (!ent) continue;
        int y = y0 + i;
        bool sel = (ent == m_selected);
        bool cur = (m_focus == 0 && i == 0); // cursor highlight on first visible
        (void)cur;

        if (sel) {
            attron(COLOR_PAIR(CP_SELECTED) | A_BOLD);
        } else {
            attron(COLOR_PAIR(CP_NORMAL));
        }
        mvprintw(y, x0 + 1, " %-*s", w - 2, ent->GetName().c_str());
        if (sel) attroff(COLOR_PAIR(CP_SELECTED) | A_BOLD);
        else     attroff(COLOR_PAIR(CP_NORMAL));
    }
}

// ── Content area ───────────────────────────────────────────────

void Editor::DrawContentArea() {
    int y0 = 2, x0 = m_leftW + 1;
    int w = m_termW - m_leftW - m_rightW - 2;
    int h = m_termH - m_logH - 3;
    if (w < 10) w = 10;

    // Border
    attron(COLOR_PAIR(m_focus == 1 ? CP_ACTIVE : CP_NORMAL));
    for (int y = y0; y < y0 + h && y < m_termH; y++) {
        mvaddch(y, x0 - 1, ACS_VLINE);
        mvaddch(y, x0 + w, ACS_VLINE);
    }
    attroff(COLOR_PAIR(m_focus == 1 ? CP_ACTIVE : CP_NORMAL));

    switch (m_contentTab) {
        case ContentTab::Info:    DrawInfoContent();    break;
        case ContentTab::Script:  DrawScriptContent();  break;
        case ContentTab::Project: DrawProjectContent(); break;
    }
}

void Editor::DrawInfoContent() {
    int y0 = 2, x0 = m_leftW + 2;
    int w = m_termW - m_leftW - m_rightW - 4;
    if (w < 5) return;

    attron(COLOR_PAIR(CP_NORMAL));
    if (!m_selected) {
        mvprintw(y0 + 1, x0, "No entity selected.");
        mvprintw(y0 + 3, x0, "Use the Scene Outliner (left panel) or press");
        mvprintw(y0 + 4, x0, "Tab to focus it, then j/k to navigate, Enter to select.");
        mvprintw(y0 + 6, x0, "Keys: c=Create Cube  s=Create Sphere  l=Create Light");
        mvprintw(y0 + 7, x0, "      d=Delete  D=Duplicate  g=Grab  r=Rotate  t=Scale");
        mvprintw(y0 + 9, x0, "Viewport: press F8 to toggle the 3D viewport window.");
        attroff(COLOR_PAIR(CP_NORMAL));
        return;
    }

    mvprintw(y0, x0, "Entity: %s", m_selected->GetName().c_str());
    auto pos = m_selected->GetPosition();
    auto euler = m_selected->GetEulerAngles();
    auto scale = m_selected->GetScale();
    mvprintw(y0 + 2, x0, "Position:  %.2f  %.2f  %.2f", pos.x, pos.y, pos.z);
    mvprintw(y0 + 3, x0, "Rotation:  %.1f  %.1f  %.1f", euler.x, euler.y, euler.z);
    mvprintw(y0 + 4, x0, "Scale:     %.2f  %.2f  %.2f", scale.x, scale.y, scale.z);
}

void Editor::DrawScriptContent() {
    int y0 = 2, x0 = m_leftW + 2;
    int w = m_termW - m_leftW - m_rightW - 4;
    int h = m_termH - m_logH - 3;
    if (w < 5 || h < 3) return;

    auto* tab = ScriptEditor::Instance().GetActiveTab();
    if (!tab) {
        attron(COLOR_PAIR(CP_NORMAL));
        mvprintw(y0 + 1, x0, "No file open.");
        mvprintw(y0 + 3, x0, "Open a .lua file with:");
        mvprintw(y0 + 4, x0, "  ScriptEditor::Instance().OpenFile(\"path\")");
        mvprintw(y0 + 6, x0, "Or press Ctrl+O in the script editor.");
        attroff(COLOR_PAIR(CP_NORMAL));
        return;
    }

    // Tab name header
    attron(A_BOLD);
    mvprintw(y0, x0, " %s%s", tab->name.c_str(), tab->dirty ? " *" : "");
    attroff(A_BOLD);

    // Line numbers + content
    auto lines = GetScriptLines();
    int maxLines = h - 2;
    if (m_scriptScrollY > (int)lines.size() - maxLines)
        m_scriptScrollY = std::max(0, (int)lines.size() - maxLines);

    for (int i = 0; i < maxLines && (m_scriptScrollY + i) < (int)lines.size(); i++) {
        int y = y0 + 1 + i;
        int lineNum = m_scriptScrollY + i;

        // Line number
        attron(COLOR_PAIR(CP_STATUS));
        mvprintw(y, x0, "%4d ", lineNum + 1);
        attroff(COLOR_PAIR(CP_STATUS));

        // Content with scroll
        const std::string& text = lines[lineNum];
        int scX = m_scriptScrollX;
        int dispLen = std::min((int)text.size() - scX, w - 6);
        if (dispLen < 0) dispLen = 0;
        std::string disp = text.substr(scX, dispLen);

        // Cursor line highlight
        bool curLine = (lineNum == m_scriptCursorY && m_focus == 1);
        if (curLine) attron(COLOR_PAIR(CP_HIGHLIGHT));

        mvprintw(y, x0 + 5, "%.*s", dispLen, disp.c_str());
        // Clear rest of line
        for (int c = x0 + 5 + dispLen; c < x0 + 5 + w - 5; c++) mvaddch(y, c, ' ');

        if (curLine) attroff(COLOR_PAIR(CP_HIGHLIGHT));
    }

    // Cursor indicator
    if (m_focus == 1) {
        int cy = y0 + 1 + (m_scriptCursorY - m_scriptScrollY);
        int cx = x0 + 5 + (m_scriptCursorX - m_scriptScrollX);
        if (cy >= y0 && cy < y0 + h && cx >= x0 + 5 && cx < x0 + 5 + w) {
            mvaddch(cy, cx, '_' | A_REVERSE);
        }
    }
}

void Editor::DrawProjectContent() {
    int y0 = 2, x0 = m_leftW + 2;
    int w = m_termW - m_leftW - m_rightW - 4;
    (void)w;

    static ProjectManifest proj;
    static bool loaded = false;
    if (!loaded) {
        proj.Load("project.planet");
        loaded = true;
    }

    attron(COLOR_PAIR(CP_NORMAL));
    mvprintw(y0,     x0, "Project: %s", proj.name.c_str());
    mvprintw(y0 + 1, x0, "Version: %s", proj.version.c_str());
    mvprintw(y0 + 2, x0, "Main:    %s", proj.mainScript.c_str());
    mvprintw(y0 + 3, x0, "Output:  %s", proj.buildOutput.c_str());
    mvprintw(y0 + 4, x0, "Cross-compile Windows: %s", proj.crossCompileWindows ? "yes" : "no");
    mvprintw(y0 + 6, x0, "Libraries (%zu):", proj.libraries.size());
    for (size_t i = 0; i < proj.libraries.size() && i < 8; i++)
        mvprintw(y0 + 7 + (int)i, x0 + 2, "%s", proj.libraries[i].c_str());
    int ly = y0 + 7 + (int)std::min(proj.libraries.size(), (size_t)8);
    mvprintw(ly,     x0, "Asset Paths (%zu):", proj.assetPaths.size());
    for (size_t i = 0; i < proj.assetPaths.size() && i < 6; i++)
        mvprintw(ly + 1 + (int)i, x0 + 2, "%s", proj.assetPaths[i].c_str());
    attroff(COLOR_PAIR(CP_NORMAL));
}

// ── Properties panel ───────────────────────────────────────────

void Editor::DrawProperties() {
    int y0 = 2, x0 = m_termW - m_rightW;
    int w = m_rightW - 1, h = m_termH - m_logH - 3;

    attron(COLOR_PAIR(m_focus == 2 ? CP_ACTIVE : CP_NORMAL));
    for (int y = y0; y < y0 + h && y < m_termH; y++) {
        mvaddch(y, x0 - 1, ACS_VLINE);
    }
    mvprintw(y0 - 1, x0 + 1, " Properties ");
    attroff(COLOR_PAIR(m_focus == 2 ? CP_ACTIVE : CP_NORMAL));

    if (!m_selected) {
        attron(COLOR_PAIR(CP_NORMAL));
        mvprintw(y0 + 1, x0 + 1, "No selection");
        attroff(COLOR_PAIR(CP_NORMAL));
        return;
    }

    int line = y0 + 1;
    attron(COLOR_PAIR(CP_NORMAL));
    mvprintw(line++, x0 + 1, "Name: %s", m_selected->GetName().c_str()); line++;
    auto pos = m_selected->GetPosition();
    auto euler = m_selected->GetEulerAngles();
    auto scale = m_selected->GetScale();
    mvprintw(line++, x0 + 1, "Pos: %.1f %.1f %.1f", pos.x, pos.y, pos.z);
    mvprintw(line++, x0 + 1, "Rot: %.1f %.1f %.1f", euler.x, euler.y, euler.z);
    mvprintw(line++, x0 + 1, "Scl: %.1f %.1f %.1f", scale.x, scale.y, scale.z);
    line++;

    auto* mc = m_selected->GetComponent<MeshComponent>();
    if (mc) {
        mvprintw(line++, x0 + 1, "Mesh: %s", mc->mesh ? "yes" : "no");
        mvprintw(line++, x0 + 1, "Tex:  %s", mc->texture ? "yes" : "no");
        mvprintw(line++, x0 + 1, "Col:  %.1f %.1f %.1f",
                 mc->color.r, mc->color.g, mc->color.b);
    }
    auto* lc = m_selected->GetComponent<LightComponent>();
    if (lc) {
        mvprintw(line++, x0 + 1, "Light: dir=%.1f %.1f %.1f",
                 lc->color.r, lc->color.g, lc->color.b);
    }
    line++;
    auto compTypes = m_selected->GetComponentTypes();
    mvprintw(line++, x0 + 1, "Comps: %zu", compTypes.size());
    for (auto& t : compTypes) {
        if (line < y0 + h - 1)
            mvprintw(line++, x0 + 2, "%s", t.name());
    }
    attroff(COLOR_PAIR(CP_NORMAL));
}

// ── Log console ────────────────────────────────────────────────

void Editor::DrawLog() {
    int y0 = m_termH - m_logH - 1;
    int h = m_logH;

    // Divider
    attron(COLOR_PAIR(m_focus == 3 ? CP_ACTIVE : CP_NORMAL));
    for (int x = 0; x < m_termW; x++) mvaddch(y0, x, ACS_HLINE);
    mvprintw(y0, 0, " Log Console ");
    attroff(COLOR_PAIR(m_focus == 3 ? CP_ACTIVE : CP_NORMAL));

    const auto& entries = LogConsole::Instance().GetEntries();
    int visLines = h - 1;
    if (m_logScroll > (int)entries.size() - visLines)
        m_logScroll = std::max(0, (int)entries.size() - visLines);

    const char* labels[] = {"[I]", "[W]", "[E]"};
    int pairs[] = {CP_INFO, CP_WARN, CP_ERROR};

    for (int i = 0; i < visLines && (m_logScroll + i) < (int)entries.size(); i++) {
        auto& entry = entries[m_logScroll + i];
        int y = y0 + 1 + i;

        int lvl = entry.level;
        if (lvl < 0) lvl = 0;
        if (lvl > 2) lvl = 2;

        attron(COLOR_PAIR(pairs[lvl]));
        mvprintw(y, 1, "%s %.*s", labels[lvl], m_termW - 6, entry.message.c_str());
        attroff(COLOR_PAIR(pairs[lvl]));
    }
}

// ── Status bar ─────────────────────────────────────────────────

void Editor::DrawStatusBar() {
    int y = m_termH - 1;
    attron(COLOR_PAIR(CP_STATUS));
    for (int x = 0; x < m_termW; x++) mvaddch(y, x, ' ');

    const char* focusNames[] = {"Scene", "Content", "Props", "Log"};
    int col = 1;
    mvprintw(y, col, "Focus: %s", focusNames[m_focus]); col += 12;
    mvprintw(y, col, "  Tab:switch  Enter:select  ESC:deselect  j/k:nav");
    attroff(COLOR_PAIR(CP_STATUS));
}

} // namespace planet
