#include "planet/editor/script_editor.h"
#include "planet/editor/log_console.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace planet {

ScriptEditor& ScriptEditor::Instance() {
    static ScriptEditor instance;
    return instance;
}

void ScriptEditor::NewFile() {
    Tab tab;
    int idx = 1;
    for (auto& t : m_tabs) {
        if (t.name.find("Untitled") == 0) idx++;
    }
    tab.name = "Untitled" + std::to_string(idx) + ".lua";
    tab.content.clear();
    tab.dirty = false;
    m_tabs.push_back(tab);
    m_activeTab = (int)m_tabs.size() - 1;
}

void ScriptEditor::OpenFile(const std::string& path) {
    for (auto& tab : m_tabs) {
        if (tab.filePath == path) {
            m_activeTab = (int)(&tab - &m_tabs[0]);
            return;
        }
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        EditorLog("Failed to open: " + path, 2);
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();

    Tab tab;
    tab.name = path.substr(path.find_last_of("/\\") + 1);
    tab.filePath = path;
    tab.content = ss.str();
    tab.dirty = false;
    m_tabs.push_back(tab);
    m_activeTab = (int)m_tabs.size() - 1;
    EditorLog("Opened: " + path, 0);
}

void ScriptEditor::CloseTab(int index) {
    if (index < 0 || index >= (int)m_tabs.size()) return;
    if (m_tabs[index].dirty)
        EditorLog("Tab closed with unsaved changes: " + m_tabs[index].name, 1);
    m_tabs.erase(m_tabs.begin() + index);
    if (m_activeTab >= (int)m_tabs.size()) m_activeTab = (int)m_tabs.size() - 1;
}

void ScriptEditor::SaveFile(int index) {
    if (index < 0 || index >= (int)m_tabs.size()) return;
    auto& tab = m_tabs[index];
    if (tab.filePath.empty()) {
        EditorLog("Save As not supported in TUI mode", 1);
        return;
    }
    std::ofstream file(tab.filePath);
    if (!file.is_open()) {
        EditorLog("Failed to save: " + tab.filePath, 2);
        return;
    }
    file << tab.content;
    tab.dirty = false;
    EditorLog("Saved: " + tab.filePath, 0);
}

void ScriptEditor::SaveCurrentFile() {
    SaveFile(m_activeTab);
}

ScriptEditor::Tab* ScriptEditor::GetActiveTab() {
    if (m_activeTab < 0 || m_activeTab >= (int)m_tabs.size()) return nullptr;
    return &m_tabs[m_activeTab];
}

} // namespace planet
