#pragma once

#include <string>
#include <vector>
#include <functional>

namespace planet {

class ScriptEditor {
public:
    struct Tab {
        std::string name;
        std::string filePath;
        std::string content;
        bool dirty = false;
    };

    static ScriptEditor& Instance();

    void OpenFile(const std::string& path);
    void NewFile();
    void CloseTab(int index);
    void SaveFile(int index);
    void SaveCurrentFile();
    void SaveAs(int index);

    void Show(bool* open = nullptr);

    Tab* GetActiveTab();
    int GetActiveTabIndex() const { return m_activeTab; }
    bool IsOpen() const { return m_activeTab >= 0 && m_activeTab < (int)m_tabs.size(); }

private:
    ScriptEditor() = default;

    void DrawSyntaxHighlighted(const std::string& text, int startLine, int endLine);
    bool MatchLuaKeyword(const char* word, int len);

    std::vector<Tab> m_tabs;
    int m_activeTab = -1;
    char m_searchBuffer[256] = {};
    char m_replaceBuffer[256] = {};
    bool m_showSearch = false;
    bool m_showReplace = false;
};

} // namespace planet
