#include "planet/editor/editor_ui.h"
#include "planet/editor/editor.h"
#include "planet/core/window.h"
#include "planet/render/sprite_renderer.h"
#include "planet/render/text_renderer.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"

#include <string>
#include <sstream>
#include <glm/glm.hpp>

namespace planet {

void RenderEditorUI() {
    auto& sr = SpriteRenderer::Instance();
    auto& tr = TextRenderer::Instance();
    auto& editor = Editor::Instance();
    float w = static_cast<float>(Window::Instance().GetWidth());
    float h = static_cast<float>(Window::Instance().GetHeight());

    // === Top toolbar ===
    float toolbarH = 32;
    sr.DrawSprite(glm::vec2(0, 0), glm::vec2(w, toolbarH), glm::vec4(0.15f, 0.15f, 0.2f, 0.95f));
    sr.DrawSprite(glm::vec2(0, toolbarH), glm::vec2(w, 1), glm::vec4(0.2f, 0.25f, 0.35f, 0.6f));

    std::string selName = "No selection";
    Entity* sel = editor.GetSelectedEntity();
    if (sel) selName = sel->GetName();

    // Mode indicator
    std::string modeStr = editor.GetModeString();
    if (editor.IsInModalTransform()) {
        modeStr += " [" + std::string(editor.GetTransformString()) + "]";
        sr.DrawSprite(glm::vec2(0, 32), glm::vec2(w, 28), glm::vec4(0.25f, 0.5f, 0.3f, 0.9f));
        tr.DrawString(modeStr, w / 2 - 60, 37, 0.8f, glm::vec4(0.9f, 1.0f, 0.9f, 1.0f));
        tr.DrawString("LMB/Enter: confirm  Esc/RMB: cancel  XYZ: axis", w / 2 + 70, 37, 0.7f, glm::vec4(0.5f, 0.8f, 0.55f, 1.0f));
    }

    tr.DrawString("Selected: " + selName, w - 260, 8, 0.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
    tr.DrawString("MMB: Orbit  S+MMB: Pan  MW: Zoom  G: Grab  R: Rotate  S: Scale", w / 2 - 260, 8, 0.7f, glm::vec4(0.5f, 0.5f, 0.55f, 1.0f));

    // === Entity tree (left) ===
    float panelW = 220;
    float treeTop = toolbarH + 2;
    float treeH = h - treeTop - 22 - 2;

    // Panel background
    sr.DrawSprite(glm::vec2(0, treeTop), glm::vec2(panelW, treeH), glm::vec4(0.08f, 0.08f, 0.1f, 0.92f));
    sr.DrawSprite(glm::vec2(0, treeTop), glm::vec2(panelW, 24), glm::vec4(0.15f, 0.15f, 0.2f, 0.95f));
    tr.DrawString("Scene Entities", 6, treeTop + 4, 0.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
    sr.DrawSprite(glm::vec2(0, treeTop + 24), glm::vec2(panelW, 1), glm::vec4(0.3f, 0.3f, 0.4f, 0.5f));

    float lineH = 18;
    int count = Editor::Instance().GetEntityCount();
    int visible = static_cast<int>((treeH - 28) / lineH);
    for (int i = 0; i < visible && i < count; i++) {
        Entity* ent = Editor::Instance().GetEntityByIndex(i);
        if (!ent) continue;
        float ey = treeTop + 28 + i * lineH;
        std::string name = ent->GetName();
        if (!ent->IsActive()) name += " [x]";
        tr.DrawString(name, 6, ey + 1, 0.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
        sr.DrawSprite(glm::vec2(0, ey + lineH), glm::vec2(panelW, 1), glm::vec4(0.12f, 0.12f, 0.14f, 0.5f));
    }

    // === Properties (right) ===
    float propX = w - panelW;
    sr.DrawSprite(glm::vec2(propX, treeTop), glm::vec2(panelW, treeH), glm::vec4(0.08f, 0.08f, 0.1f, 0.92f));
    sr.DrawSprite(glm::vec2(propX, treeTop), glm::vec2(panelW, 24), glm::vec4(0.15f, 0.15f, 0.2f, 0.95f));
    tr.DrawString("Properties", propX + 6, treeTop + 4, 0.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
    sr.DrawSprite(glm::vec2(propX, treeTop + 24), glm::vec2(panelW, 1), glm::vec4(0.3f, 0.3f, 0.4f, 0.5f));

    if (sel) {
        auto pos = sel->GetPosition();
        auto euler = sel->GetEulerAngles();
        auto s = sel->GetScale();
        float px = propX + 6;
        float py = treeTop + 30;
        auto dim = glm::vec4(0.5f, 0.5f, 0.55f, 1.0f);
        auto acc = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);

        tr.DrawString("Name", px, py, 0.8f, dim);
        tr.DrawString(sel->GetName(), px + panelW - 80, py, 0.8f, acc);
        py += 16;

        tr.DrawString("Position", px, py, 0.8f, glm::vec4(0.8f, 0.8f, 0.85f, 1.0f));
        py += 16;
        auto fmt = [](float v) { std::string s = std::to_string(v); return s.substr(0, s.find('.') + 3); };
        tr.DrawString("X", px, py, 0.8f, dim);
        tr.DrawString(fmt(pos.x), px + panelW - 70, py, 0.8f, acc);
        py += 16;
        tr.DrawString("Y", px, py, 0.8f, dim);
        tr.DrawString(fmt(pos.y), px + panelW - 70, py, 0.8f, acc);
        py += 16;
        tr.DrawString("Z", px, py, 0.8f, dim);
        tr.DrawString(fmt(pos.z), px + panelW - 70, py, 0.8f, acc);
        py += 24;

        tr.DrawString("Rotation", px, py, 0.8f, glm::vec4(0.8f, 0.8f, 0.85f, 1.0f));
        py += 16;
        auto f1 = [](float v) { std::string s = std::to_string(v); return s.substr(0, s.find('.') + 2); };
        tr.DrawString("X " + f1(euler.x), px, py, 0.8f, acc);
        py += 16;
        tr.DrawString("Y " + f1(euler.y), px, py, 0.8f, acc);
        py += 16;
        tr.DrawString("Z " + f1(euler.z), px, py, 0.8f, acc);
        py += 24;

        tr.DrawString("Scale", px, py, 0.8f, glm::vec4(0.8f, 0.8f, 0.85f, 1.0f));
        py += 16;
        tr.DrawString(fmt(s.x), px, py, 0.8f, acc);
        py += 16;
        tr.DrawString(fmt(s.y), px, py, 0.8f, acc);
        py += 16;
        tr.DrawString(fmt(s.z), px, py, 0.8f, acc);
    } else {
        tr.DrawString("No entity selected", propX + 6, treeTop + 30, 0.8f, glm::vec4(0.5f, 0.5f, 0.55f, 1.0f));
    }

    // === Bottom status bar ===
    float barH = 20;
    float barY = h - barH;
    sr.DrawSprite(glm::vec2(0, barY), glm::vec2(w, barH), glm::vec4(0.15f, 0.15f, 0.2f, 0.95f));
    std::stringstream ss;
    ss << count << " entities  |  Grid snap: " << Editor::Instance().GetGridSnap();
    tr.DrawString(ss.str(), 8, barY + 2, 0.7f, glm::vec4(0.5f, 0.5f, 0.55f, 1.0f));

    auto target = Editor::Instance().GetCameraTarget();
    std::stringstream ss2;
    ss2 << "Target: " << target.x << " " << target.y << " " << target.z;
    tr.DrawString(ss2.str(), w - 280, barY + 2, 0.7f, glm::vec4(0.5f, 0.5f, 0.55f, 1.0f));
}

} // namespace planet
