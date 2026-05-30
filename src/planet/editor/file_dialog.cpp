#include "planet/editor/file_dialog.h"
#include "planet/editor/log_console.h"
#include <algorithm>
#include <filesystem>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

namespace fs = std::filesystem;

namespace planet {

FileDialog& FileDialog::Instance() {
    static FileDialog instance;
    return instance;
}

void FileDialog::Open(const std::string& title, const std::string& startPath,
                      const std::string& filter, bool saveMode,
                      std::function<void(const FileDialogResult&)> callback) {
    m_title = title;
    m_filter = filter;
    m_saveMode = saveMode;
    m_callback = callback;
    m_fileNameBuf[0] = 0;
    m_result.clear();
    std::string start = startPath.empty() ? fs::current_path().string() : startPath;
    // Navigate to start path
    EditorLog("File dialog: " + title + " in " + fs::absolute(start).string(), 0);
    (void)m_filter;
    (void)m_saveMode;
    m_open = true;
    m_result = start + "/" + (m_fileNameBuf[0] ? m_fileNameBuf : "");
    FileDialogResult res;
    res.confirmed = true;
    res.path = m_result;
    if (m_callback) m_callback(res);
    m_open = false;
}

} // namespace planet
