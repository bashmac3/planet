#include "planet/mapper/mapper.h"
#include "planet/core/scene.h"
#include "planet/core/camera.h"
#include "planet/render/renderer.h"
#include "planet/render/model_renderer.h"
#include "planet/render/mesh.h"
#include "planet/render/framebuffer.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/ecs/system.h"
#include "planet/resource/resource_manager.h"
#include "planet/core/logger.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <lua.hpp>
#include <lauxlib.h>
#include <algorithm>
#include <clocale>
#include <cstring>
#include <fstream>
#include <set>

namespace planet {

// ── Singleton ──────────────────────────────────────────────────

Mapper& Mapper::Instance() {
    static Mapper inst;
    return inst;
}

// ── Init / Shutdown ────────────────────────────────────────────

void Mapper::Init(int argc, char** argv) {
    m_path = (argc > 1) ? argv[1] : "maps/demo.lua";

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(16);
    curs_set(0);
    start_color();

    mousemask(BUTTON1_CLICKED | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    printf("\033[?1002h\033[?1006h");

    init_pair(CP_NORMAL, COLOR_WHITE,   COLOR_BLACK);
    init_pair(CP_SEL,    COLOR_BLACK,   COLOR_CYAN);
    init_pair(CP_HDR,    COLOR_WHITE,   COLOR_BLUE);
    init_pair(CP_STAT,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(CP_ACT,    COLOR_BLACK,   COLOR_CYAN);
    init_pair(CP_TITLE,  COLOR_YELLOW,  COLOR_BLUE);
    init_pair(CP_INPUT,  COLOR_WHITE,   COLOR_BLUE);

    LoadFromLua(m_path);
    PopulateScene();
}

void Mapper::Shutdown() {
    printf("\033[?1002l\033[?1006l");
    m_fbo.reset(); m_cam.reset();
    curs_set(1);
    endwin();
}

// ── Main loop ──────────────────────────────────────────────────

void Mapper::Run() {
    int lw = 0, lh = 0;
    while (true) {
        getmaxyx(stdscr, m_termH, m_termW);
        if (m_termW != lw || m_termH != lh) { lw = m_termW; lh = m_termH; }
        DrawAll();
        doupdate();
        if (m_vpVis) RenderViewport();
        glfwPollEvents();
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
        if (ch != ERR) HandleInput(ch);
    }
}

// ── Drawing ────────────────────────────────────────────────────

void Mapper::DrawAll() {
    erase();
    DrawHeader();
    DrawList();
    DrawDetail();
    DrawStatus();
}

void Mapper::DrawHeader() {
    attron(COLOR_PAIR(CP_HDR) | A_BOLD);
    for (int x = 0; x < m_termW; x++) mvaddch(0, x, ' ');
    const char* file = m_path.c_str();
    int flen = (int)std::strlen(file);
    if (flen > m_termW - 20) file += flen - (m_termW - 20);
    mvprintw(0, 1, " %s%s ", file, m_dirty ? " *" : "");

    int x = std::max(flen + 6, m_termW - 38);
    mvprintw(0, x, " %s ", m_mode == MODE_ENV ? "[Env]" : " Env ");
    x += 6;
    mvprintw(0, x, " %s ", m_mode == MODE_TPL ? "[Tpl]" : " Tpl ");
    attroff(COLOR_PAIR(CP_HDR) | A_BOLD);
}

void Mapper::DrawList() {
    int w = std::max(20, m_termW * 30 / 100);
    int h = m_termH - 2;
    int x0 = 0, y0 = 1;

    attron(COLOR_PAIR(CP_ACT));
    for (int y = y0; y < y0 + h && y < m_termH; y++) mvaddch(y, x0 + w, ' ');
    mvprintw(y0, x0 + 1, " %s ", m_mode == MODE_ENV ? "Environment" : "Templates");
    attroff(COLOR_PAIR(CP_ACT));

    int vis = h - 1;
    int total = (m_mode == MODE_ENV) ? (int)m_env.size() : (int)m_tpl.size();
    m_scroll = std::min(m_scroll, std::max(0, total - vis));
    m_scroll = std::max(0, m_scroll);

    for (int i = 0; i < vis && (m_scroll + i) < total; i++) {
        int idx = m_scroll + i;
        int y = y0 + 1 + i;
        bool cur = (idx == m_sel);

        if (cur) attron(COLOR_PAIR(CP_SEL) | A_BOLD);
        else     attron(COLOR_PAIR(CP_NORMAL));

        std::string label;
        if (m_mode == MODE_ENV)
            label = m_env[idx].name.empty() ? "(unnamed)" : m_env[idx].name;
        else
            label = m_tpl[idx].name.empty() ? "(unnamed)" : m_tpl[idx].name;

        mvprintw(y, x0 + 1, " %-*s", w - 2, label.c_str());
        if (cur) attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
        else     attroff(COLOR_PAIR(CP_NORMAL));
    }
}

void Mapper::DrawDetail() {
    int x0 = std::max(20, m_termW * 30 / 100) + 2;
    int w = m_termW - x0 - 1;
    int h = m_termH - 2;
    if (w < 10) return;

    int total = (m_mode == MODE_ENV) ? (int)m_env.size() : (int)m_tpl.size();
    if (m_sel < 0 || m_sel >= total) {
        attron(COLOR_PAIR(CP_NORMAL));
        mvprintw(2, x0, "No item selected.");
        mvprintw(4, x0, "n: new item    d: delete");
        mvprintw(5, x0, "s: save    F8: viewport");
        attroff(COLOR_PAIR(CP_NORMAL));
        return;
    }

    attron(COLOR_PAIR(CP_NORMAL));
    int y = 2;
    auto drawField = [&](const char* label, const std::string& val) {
        if (y >= m_termH - 2) return;
        mvprintw(y++, x0, "%s: %s", label, val.c_str());
    };
    auto drawVec3 = [&](const char* label, const glm::vec3& v) {
        if (y >= m_termH - 2) return;
        mvprintw(y++, x0, "%s: %.2f %.2f %.2f", label, v.x, v.y, v.z);
    };
    auto drawVec4 = [&](const char* label, const glm::vec4& v) {
        if (y >= m_termH - 2) return;
        mvprintw(y++, x0, "%s: %.2f %.2f %.2f %.2f", label, v.x, v.y, v.z, v.w);
    };

    if (m_mode == MODE_ENV) {
        auto& e = m_env[m_sel];
        drawField("Name", e.name);
        drawField("Type", e.type);
        if (e.type == "light") {
            drawField("  lightType", e.lightType);
            drawVec3("  color", e.lightColor);
            char buf[32]; snprintf(buf, sizeof(buf), "%.1f", e.lightRange);
            drawField("  range", buf);
        }
        if (e.type == "mesh") {
            drawField("  mesh", e.meshType);
            drawVec4("  color", e.meshColor);
            drawVec3("  scale", e.scale);
            drawVec3("  position", e.position);
            drawVec3("  rotation", e.rotation);
            drawField("  texture", e.texture);
        }
    } else {
        auto& t = m_tpl[m_sel];
        drawField("Name", t.name);
        if (t.isLight) {
            drawField("  lightType", t.lightType);
            drawVec3("  color", t.lightColor);
            char buf[32]; snprintf(buf, sizeof(buf), "%.1f", t.lightRange);
            drawField("  range", buf);
        } else {
            drawField("  mesh", t.meshType);
            drawVec4("  color", t.color);
            drawVec3("  scale", t.scale);
            drawField("  texture", t.texture);
        }
    }
    attroff(COLOR_PAIR(CP_NORMAL));
}

void Mapper::DrawStatus() {
    int y = m_termH - 1;
    attron(COLOR_PAIR(CP_STAT));
    for (int x = 0; x < m_termW; x++) mvaddch(y, x, ' ');
    const char* modeName = (m_mode == MODE_ENV) ? "Environment" : "Templates";
    int n = (m_mode == MODE_ENV) ? (int)m_env.size() : (int)m_tpl.size();
    mvprintw(y, 1, "%s [%d]  n:new  d:del  Tab:mode  Enter:edit  s:save  F8:3D  q:quit",
             modeName, n);
    attroff(COLOR_PAIR(CP_STAT));
}

// ── Input ──────────────────────────────────────────────────────

void Mapper::HandleInput(int ch) {
    if (ch == '\t') {
        m_mode = (m_mode == MODE_ENV) ? MODE_TPL : MODE_ENV;
        m_sel = 0; m_scroll = 0;
        return;
    }
    if (ch == KEY_F(8)) { ToggleViewport(); return; }
    if (ch == 's') { if (SaveToLua()) m_dirty = false; return; }

    int n = (m_mode == MODE_ENV) ? (int)m_env.size() : (int)m_tpl.size();

    if (ch == 'n') {
        if (m_mode == MODE_ENV) AddEnv();
        else AddTpl();
        m_sel = std::max(0, n);
        m_dirty = true;
        PopulateScene();
        return;
    }

    if (ch == 'd' && n > 0 && m_sel >= 0 && m_sel < n) {
        DeleteCurrent();
        return;
    }

    if (ch == KEY_DOWN || ch == 'j') {
        if (m_sel < n - 1) m_sel++;
        if (m_sel >= m_scroll + (m_termH - 4)) m_scroll++;
        return;
    }
    if (ch == KEY_UP || ch == 'k') {
        if (m_sel > 0) m_sel--;
        if (m_sel < m_scroll) m_scroll--;
        return;
    }

    if (ch == '\n' && n > 0 && m_sel >= 0 && m_sel < n) {
        m_dirty = true;
        if (m_mode == MODE_ENV) {
            auto& e = m_env[m_sel];
            InputBox("Name", e.name);
            InputBox("Type (light|mesh)", e.type);
            if (e.type == "light") {
                InputBox("  lightType (directional|point|spot)", e.lightType);
                EditFloat3("  color r g b", e.lightColor);
                std::string rs = std::to_string(e.lightRange);
                InputBox("  range", rs);
                e.lightRange = std::stof(rs);
            }
            if (e.type == "mesh") {
                InputBox("  mesh (box|sphere|plane|cylinder|cone|torus)", e.meshType);
                EditFloat4("  color r g b a", e.meshColor);
                EditFloat3("  scale x y z", e.scale);
                EditFloat3("  position x y z", e.position);
                EditFloat3("  rotation pitch yaw roll", e.rotation);
                InputBox("  texture path", e.texture);
            }
        } else {
            auto& t = m_tpl[m_sel];
            InputBox("Name", t.name);
            std::string isl = t.isLight ? "yes" : "no";
            InputBox("Is light? (yes|no)", isl);
            t.isLight = (isl == "yes");
            if (t.isLight) {
                InputBox("  lightType (directional|point|spot)", t.lightType);
                EditFloat3("  color r g b", t.lightColor);
                std::string rs = std::to_string(t.lightRange);
                InputBox("  range", rs);
                t.lightRange = std::stof(rs);
            } else {
                InputBox("  mesh (box|sphere|plane|cylinder|cone|torus)", t.meshType);
                EditFloat4("  color r g b a", t.color);
                EditFloat3("  scale x y z", t.scale);
                InputBox("  texture path", t.texture);
            }
        }
        PopulateScene();
        return;
    }

    // Mouse
    if (ch == KEY_MOUSE) {
        MEVENT ev;
        if (getmouse(&ev) == OK) {
            int w = std::max(20, m_termW * 30 / 100);
            if (ev.x >= 0 && ev.x < w && ev.y >= 1 && ev.y < m_termH - 1) {
                int idx = (ev.y - 2) + m_scroll;
                if (idx >= 0 && idx < n) m_sel = idx;
            }
        }
    }
}

// ── Map loading ────────────────────────────────────────────────

void Mapper::LoadFromLua(const std::string& path) {
    m_env.clear();
    m_tpl.clear();

    lua_State* L = luaL_newstate();
    if (!L) return;
    luaL_openlibs(L);

    if (luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, 1, 0)) {
        lua_close(L);
        LOG_INFO() << "[Mapper] Could not load " << path << ", starting empty";
        return;
    }
    if (!lua_istable(L, -1)) { lua_close(L); return; }

    lua_getfield(L, -1, "environment");
    if (lua_istable(L, -1)) ParseEnvTable(L, lua_gettop(L));
    lua_pop(L, 1);

    lua_getfield(L, -1, "templates");
    if (lua_istable(L, -1)) ParseTplTable(L, lua_gettop(L));
    lua_pop(L, 1);

    lua_close(L);
    LOG_INFO() << "[Mapper] Loaded " << path;
}

void Mapper::ParseEnvTable(lua_State* L, int idx) {
    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
        if (!lua_istable(L, -1)) { lua_pop(L, 1); continue; }
        EnvEntry e;

        lua_getfield(L, -1, "name");
        if (lua_isstring(L, -1)) e.name = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "type");
        if (lua_isstring(L, -1)) e.type = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "lightType");
        if (lua_isstring(L, -1)) e.lightType = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "color");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); e.lightColor.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); e.lightColor.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); e.lightColor.z = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 4); e.meshColor.a = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            e.meshColor.r = e.lightColor.x;
            e.meshColor.g = e.lightColor.y;
            e.meshColor.b = e.lightColor.z;
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "range");
        if (lua_isnumber(L, -1)) e.lightRange = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "mesh");
        if (lua_isstring(L, -1)) e.meshType = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "scale");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); e.scale.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); e.scale.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); e.scale.z = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "position");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); e.position.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); e.position.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); e.position.z = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "rotation");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); e.rotation.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); e.rotation.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); e.rotation.z = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "texture");
        if (lua_isstring(L, -1)) e.texture = lua_tostring(L, -1);
        lua_pop(L, 1);

        m_env.push_back(e);
        lua_pop(L, 1);
    }
}

void Mapper::ParseTplTable(lua_State* L, int idx) {
    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
        if (!lua_istable(L, -1)) { lua_pop(L, 1); continue; }
        TplEntry t;
        const char* tplName = lua_tostring(L, -2);
        if (tplName) t.name = tplName;

        lua_getfield(L, -1, "mesh");
        if (lua_isstring(L, -1)) { t.meshType = lua_tostring(L, -1); }
        lua_pop(L, 1);

        lua_getfield(L, -1, "lightType");
        if (lua_isstring(L, -1)) { t.lightType = lua_tostring(L, -1); t.isLight = true; }
        lua_pop(L, 1);

        lua_getfield(L, -1, "color");
        if (lua_istable(L, -1)) {
            float r=1,g=1,b=1,a=1;
            lua_rawgeti(L, -1, 1); r=(float)lua_tonumber(L,-1); lua_pop(L,1);
            lua_rawgeti(L, -1, 2); g=(float)lua_tonumber(L,-1); lua_pop(L,1);
            lua_rawgeti(L, -1, 3); b=(float)lua_tonumber(L,-1); lua_pop(L,1);
            lua_rawgeti(L, -1, 4); a=(float)lua_tonumber(L,-1); lua_pop(L,1);
            t.color = glm::vec4(r,g,b,a);
            t.lightColor = glm::vec3(r,g,b);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "scale");
        if (lua_istable(L, -1)) {
            float sx=1,sy=1,sz=1;
            lua_rawgeti(L, -1, 1); sx=(float)lua_tonumber(L,-1); lua_pop(L,1);
            lua_rawgeti(L, -1, 2); sy=(float)lua_tonumber(L,-1); lua_pop(L,1);
            lua_rawgeti(L, -1, 3); sz=(float)lua_tonumber(L,-1); lua_pop(L,1);
            t.scale = glm::vec3(sx,sy,sz);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "texture");
        if (lua_isstring(L, -1)) t.texture = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "lightRange");
        if (lua_isnumber(L, -1)) t.lightRange = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        m_tpl.push_back(t);
        lua_pop(L, 1);
    }
}

bool Mapper::SaveToLua() {
    std::string out;
    out = "return {\n";

    // Environment
    out += "    environment = {\n";
    for (auto& e : m_env) {
        out += "        {\n";
        auto esc = [](const std::string& s) { return s; };
        out += "            name = \"" + esc(e.name) + "\",\n";
        out += "            type = \"" + esc(e.type) + "\",\n";
        if (e.type == "light") {
            out += "            lightType = \"" + esc(e.lightType) + "\",\n";
            out += "            color = { " + std::to_string(e.lightColor.x) + ", "
                                        + std::to_string(e.lightColor.y) + ", "
                                        + std::to_string(e.lightColor.z) + " },\n";
            out += "            range = " + std::to_string(e.lightRange) + ",\n";
        }
        if (e.type == "mesh") {
            out += "            mesh = \"" + esc(e.meshType) + "\",\n";
            out += "            color = { " + std::to_string(e.meshColor.r) + ", "
                                        + std::to_string(e.meshColor.g) + ", "
                                        + std::to_string(e.meshColor.b) + ", "
                                        + std::to_string(e.meshColor.a) + " },\n";
            out += "            scale = { " + std::to_string(e.scale.x) + ", "
                                        + std::to_string(e.scale.y) + ", "
                                        + std::to_string(e.scale.z) + " },\n";
            if (e.position.x != 0 || e.position.y != 0 || e.position.z != 0) {
                out += "            position = { " + std::to_string(e.position.x) + ", "
                                                  + std::to_string(e.position.y) + ", "
                                                  + std::to_string(e.position.z) + " },\n";
            }
            if (e.rotation.x != 0 || e.rotation.y != 0 || e.rotation.z != 0) {
                out += "            rotation = { " + std::to_string(e.rotation.x) + ", "
                                                  + std::to_string(e.rotation.y) + ", "
                                                  + std::to_string(e.rotation.z) + " },\n";
            }
            if (!e.texture.empty())
                out += "            texture = \"" + esc(e.texture) + "\",\n";
        }
        out += "        },\n";
    }
    out += "    },\n";

    // Templates
    out += "    templates = {\n";
    for (auto& t : m_tpl) {
        out += "        " + t.name + " = {\n";
        if (t.isLight) {
            out += "            lightType = \"" + t.lightType + "\",\n";
            out += "            color = { " + std::to_string(t.lightColor.x) + ", "
                                        + std::to_string(t.lightColor.y) + ", "
                                        + std::to_string(t.lightColor.z) + " },\n";
            out += "            lightRange = " + std::to_string(t.lightRange) + ",\n";
        } else {
            out += "            type = \"mesh\",\n";
            out += "            mesh = \"" + t.meshType + "\",\n";
            out += "            color = { " + std::to_string(t.color.r) + ", "
                                        + std::to_string(t.color.g) + ", "
                                        + std::to_string(t.color.b) + ", "
                                        + std::to_string(t.color.a) + " },\n";
            out += "            scale = { " + std::to_string(t.scale.x) + ", "
                                        + std::to_string(t.scale.y) + ", "
                                        + std::to_string(t.scale.z) + " },\n";
            if (!t.texture.empty())
                out += "            texture = \"" + t.texture + "\",\n";
        }
        out += "        },\n";
    }
    out += "    },\n";
    out += "}\n";

    std::ofstream f(m_path);
    if (!f) { LOG_INFO() << "[Mapper] Failed to write " << m_path; return false; }
    f << out;
    LOG_INFO() << "[Mapper] Saved " << m_path;
    return true;
}

// ── Item operations ────────────────────────────────────────────

void Mapper::AddEnv() {
    EnvEntry e;
    e.name = "NewObject";
    e.type = "mesh";
    e.meshType = "box";
    m_env.push_back(e);
}

void Mapper::AddTpl() {
    TplEntry t;
    t.name = "new_template";
    t.meshType = "box";
    m_tpl.push_back(t);
}

void Mapper::DeleteCurrent() {
    if (m_mode == MODE_ENV && m_sel >= 0 && m_sel < (int)m_env.size()) {
        m_env.erase(m_env.begin() + m_sel);
        m_dirty = true;
        if (m_sel >= (int)m_env.size()) m_sel = (int)m_env.size() - 1;
        PopulateScene();
    }
    if (m_mode == MODE_TPL && m_sel >= 0 && m_sel < (int)m_tpl.size()) {
        m_tpl.erase(m_tpl.begin() + m_sel);
        m_dirty = true;
        if (m_sel >= (int)m_tpl.size()) m_sel = (int)m_tpl.size() - 1;
    }
}

// ── Input helpers ──────────────────────────────────────────────

void Mapper::InputBox(const std::string& prompt, std::string& out) {
    echo();
    curs_set(1);
    int w = std::min(m_termW - 4, 60);
    int x = (m_termW - w) / 2;
    int y = m_termH / 2 - 2;

    WINDOW* win = newwin(5, w, y, x);
    wbkgd(win, COLOR_PAIR(CP_INPUT));
    box(win, 0, 0);
    mvwprintw(win, 1, 2, " %s ", prompt.c_str());

    char buf[256];
    std::strncpy(buf, out.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    echo();
    mvwgetnstr(win, 2, 2, buf, sizeof(buf) - 1);
    noecho();

    delwin(win);
    touchwin(stdscr);
    curs_set(0);
    out = buf;
}

void Mapper::EditFloat3(const std::string& prompt, glm::vec3& v) {
    std::string s = std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z);
    InputBox(prompt, s);
    float x,y,z;
    if (sscanf(s.c_str(), "%f %f %f", &x, &y, &z) == 3)
        v = glm::vec3(x,y,z);
}

void Mapper::EditFloat4(const std::string& prompt, glm::vec4& v) {
    std::string s = std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w);
    InputBox(prompt, s);
    float x,y,z,w;
    if (sscanf(s.c_str(), "%f %f %f %f", &x, &y, &z, &w) == 4)
        v = glm::vec4(x,y,z,w);
}

// ── 3D Viewport ────────────────────────────────────────────────

void Mapper::ToggleViewport() {
    m_vpVis = !m_vpVis;
    if (m_vpVis) {
        if (!m_window) {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            m_window = glfwCreateWindow(640, 480, "Mapper - 3D Viewport", nullptr, nullptr);
            if (m_window) {
                glfwMakeContextCurrent(m_window);
                glfwSwapInterval(0);
                glfwShowWindow(m_window);
                m_fbo = std::make_unique<Framebuffer>();
                m_fbo->Init(128, 128);
                m_cam = std::make_unique<Camera>();
                m_cam->SetFarPlane(1000.0f);
            }
        } else {
            glfwShowWindow(m_window);
        }
    } else if (m_window) {
        glfwHideWindow(m_window);
    }
}

void Mapper::RenderViewport() {
    if (!m_vpVis || !m_window) return;
    if (glfwWindowShouldClose(m_window)) { m_vpVis = false; glfwHideWindow(m_window); return; }

    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    if (fbW < 1 || fbH < 1) return;

    if (fbW != m_vpW || fbH != m_vpH) {
        m_fbo->Init(fbW, fbH);
        m_vpW = fbW; m_vpH = fbH;
    }

    auto& renderer = Renderer::Instance();
    Camera* gc = Scene::Instance().GetActiveCamera();
    if (!gc) return;

    m_cam->SetPosition(glm::vec3(10,8,10));
    m_cam->SetTarget(glm::vec3(0,0,0));
    m_cam->SetFOV(gc->GetFOV());

    m_fbo->Bind();
    glViewport(0,0,fbW,fbH);
    renderer.ClearColor(0.1f,0.1f,0.15f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.SetViewMatrix(m_cam->GetViewMatrix());
    renderer.SetProjectionMatrix(m_cam->GetProjectionMatrix((float)fbW/(float)fbH));
    ModelRenderer::Instance().BeginFrame();
    SystemManager::Instance().UpdateAll(0.0);
    ModelRenderer::Instance().EndFrame();
    m_fbo->Unbind();

    glad_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glad_glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->GetFboId());
    glad_glBlitFramebuffer(0,0,fbW,fbH,0,0,fbW,fbH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glfwSwapBuffers(m_window);
}

void Mapper::PopulateScene() {
    auto& scene = Scene::Instance();
    scene.Clear();

    // Create main camera
    auto cam = std::make_unique<Camera>();
    cam->SetPosition(glm::vec3(8,6,8));
    cam->SetTarget(glm::vec3(0,0,0));
    cam->SetFOV(60.0f);
    scene.SetActiveCamera(std::move(cam));

    // Shared meshes cache
    static std::unique_ptr<Mesh> s_cube, s_sphere, s_plane, s_cyl, s_cone, s_torus;
    if (!s_cube)   s_cube   = std::make_unique<Mesh>(Mesh::CreateCube(1.0f));
    if (!s_sphere) s_sphere = std::make_unique<Mesh>(Mesh::CreateSphere(0.5f, 16));
    if (!s_plane)  s_plane  = std::make_unique<Mesh>(Mesh::CreatePlane(1, 1));
    if (!s_cyl)    s_cyl    = std::make_unique<Mesh>(Mesh::CreateCylinder(0.5f, 1.0f, 16));
    if (!s_cone)   s_cone   = std::make_unique<Mesh>(Mesh::CreateCone(0.5f, 1.0f, 16));
    if (!s_torus)  s_torus  = std::make_unique<Mesh>(Mesh::CreateTorus(0.5f, 0.2f, 16, 8));

    auto getMesh = [&](const std::string& type) -> Mesh* {
        if (type == "cube" || type == "box")     return s_cube.get();
        if (type == "sphere")                     return s_sphere.get();
        if (type == "plane")                      return s_plane.get();
        if (type == "cylinder")                   return s_cyl.get();
        if (type == "cone")                       return s_cone.get();
        if (type == "torus")                      return s_torus.get();
        return s_cube.get();
    };

    // Spawn environment meshes
    for (auto& e : m_env) {
        if (e.type == "mesh") {
            Entity* ent = scene.CreateEntity(e.name.empty() ? "env" : e.name);
            auto* mc = ent->AddComponent<MeshComponent>();
            mc->mesh = getMesh(e.meshType);
            mc->color = e.meshColor;
            ent->SetPosition(e.position);
            ent->SetScale(e.scale);
            ent->SetEulerAngles(e.rotation);
        }
        if (e.type == "light") {
            Entity* ent = scene.CreateEntity(e.name.empty() ? "light" : e.name);
            auto* lc = ent->AddComponent<LightComponent>();
            if (e.lightType == "directional") lc->type = LightComponent::Directional;
            else if (e.lightType == "spot") lc->type = LightComponent::Spot;
            else lc->type = LightComponent::Point;
            lc->color = e.lightColor;
            lc->range = e.lightRange;
        }
    }
}

} // namespace planet
