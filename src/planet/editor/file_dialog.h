#pragma once

#include <string>
#include <vector>
#include <functional>

namespace planet {

struct FileDialogResult {
    bool confirmed = false;
    std::string path;
    std::string filter;
};

class FileDialog {
public:
    static FileDialog& Instance();

    void Open(const std::string& title, const std::string& startPath,
              const std::string& filter, bool saveMode = false,
              std::function<void(const FileDialogResult&)> callback = nullptr);
    void Close();
    void Show();

    bool IsOpen() const { return m_open; }
    const std::string& GetResult() const { return m_result; }

private:
    FileDialog() = default;

    void Navigate(const std::string& path);
    void Refresh();

    bool m_open = false;
    bool m_saveMode = false;
    std::string m_title;
    std::string m_currentPath;
    std::string m_filter;
    std::string m_result;
    char m_fileNameBuf[256] = {};
    std::vector<std::string> m_entries;
    std::vector<bool> m_isDir;
    std::function<void(const FileDialogResult&)> m_callback;
};

} // namespace planet
