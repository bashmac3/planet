#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <functional>
#include <map>

namespace planet {

struct LspDiagnostic {
    int line;
    int column;
    int endLine;
    int endColumn;
    std::string message;
    std::string severity; // "Error", "Warning", "Information", "Hint"
};

struct LspCompletionItem {
    std::string label;
    std::string detail;
    std::string insertText;
};

class LspClient {
public:
    static LspClient& Instance();

    bool Start(const std::string& serverPath, const std::string& rootUri);
    void Stop();

    void OpenDocument(const std::string& uri, const std::string& languageId, const std::string& text);
    void ChangeDocument(const std::string& uri, const std::string& text);
    void CloseDocument(const std::string& uri);
    void RequestCompletions(const std::string& uri, int line, int column);

    bool IsRunning() const { return m_running; }
    std::vector<LspDiagnostic> GetDiagnostics(const std::string& uri);
    std::vector<LspCompletionItem> GetCompletions();

    void SetDiagnosticsCallback(std::function<void(const std::string& uri, const std::vector<LspDiagnostic>&)> cb) {
        m_diagnosticsCallback = cb;
    }

private:
    LspClient() = default;
    ~LspClient();
    LspClient(const LspClient&) = delete;
    LspClient& operator=(const LspClient&) = delete;

    void SendMessage(const std::string& json);
    void ReaderThread();
    void ProcessMessage(const std::string& json);
    int m_nextId = 1;

    std::string SendRequest(const std::string& method, const std::string& params);
    std::string m_serverPath;
    std::string m_rootUri;

    int m_pipeStdin = -1;
    int m_pipeStdout = -1;
    bool m_running = false;
    std::thread m_readerThread;
    std::mutex m_mutex;
    std::string m_readBuffer;

    std::map<std::string, std::vector<LspDiagnostic>> m_diagnostics;
    std::vector<LspCompletionItem> m_completions;
    std::function<void(const std::string& uri, const std::vector<LspDiagnostic>&)> m_diagnosticsCallback;
};

} // namespace planet
