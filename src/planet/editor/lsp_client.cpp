#include "planet/editor/lsp_client.h"
#include "planet/editor/log_console.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <sstream>

namespace planet {

LspClient& LspClient::Instance() {
    static LspClient instance;
    return instance;
}

LspClient::~LspClient() {
    Stop();
}

bool LspClient::Start(const std::string& serverPath, const std::string& rootUri) {
    if (m_running) Stop();

    m_serverPath = serverPath;
    m_rootUri = rootUri;

    int stdinPipe[2], stdoutPipe[2];
    if (pipe(stdinPipe) != 0 || pipe(stdoutPipe) != 0) {
        EditorLog("LSP: Failed to create pipes", 2);
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        EditorLog("LSP: Fork failed", 2);
        close(stdinPipe[0]); close(stdinPipe[1]);
        close(stdoutPipe[0]); close(stdoutPipe[1]);
        return false;
    }

    if (pid == 0) {
        // Child: LSP server
        close(stdinPipe[1]);  // Close write end of stdin pipe
        close(stdoutPipe[0]); // Close read end of stdout pipe

        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);

        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        // Close all other FDs
        for (int i = 3; i < 1024; i++) close(i);

        execlp(serverPath.c_str(), serverPath.c_str(), nullptr);
        _exit(1);
    }

    // Parent
    close(stdinPipe[0]);  // Close read end of stdin pipe
    close(stdoutPipe[1]); // Close write end of stdout pipe

    m_pipeStdin = stdinPipe[1];
    m_pipeStdout = stdoutPipe[0];
    m_running = true;

    // Start reader thread
    m_readerThread = std::thread(&LspClient::ReaderThread, this);

    // Send initialize request
    std::string params = R"({
        "processId": )" + std::to_string(getpid()) + R"(,
        "rootUri": ")" + rootUri + R"(",
        "capabilities": {
            "textDocument": {
                "completion": {"dynamicRegistration": false},
                "diagnostics": {"dynamicRegistration": false}
            }
        }
    })";

    SendRequest("initialize", params);

    // Send initialized notification
    std::string initNotif = R"({"jsonrpc":"2.0","method":"initialized","params":{}})";
    SendMessage(initNotif);

    EditorLog("LSP server started: " + serverPath, 0);
    return true;
}

void LspClient::Stop() {
    if (!m_running) return;
    m_running = false;

    if (m_pipeStdin >= 0) {
        // Send shutdown
        std::string shutdown = R"({"jsonrpc":"2.0","id":)" + std::to_string(m_nextId++) + R"(,"method":"shutdown","params":null})";
        write(m_pipeStdin, shutdown.c_str(), shutdown.size());
        close(m_pipeStdin);
        m_pipeStdin = -1;
    }

    if (m_readerThread.joinable())
        m_readerThread.join();

    if (m_pipeStdout >= 0) {
        close(m_pipeStdout);
        m_pipeStdout = -1;
    }

    EditorLog("LSP server stopped", 0);
}

void LspClient::SendMessage(const std::string& json) {
    if (m_pipeStdin < 0) return;
    std::string header = "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n";
    std::string msg = header + json;
    write(m_pipeStdin, msg.c_str(), msg.size());
}

void LspClient::OpenDocument(const std::string& uri, const std::string& languageId, const std::string& text) {
    std::string escaped;
    for (char c : text) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }

    std::string params = R"({
        "textDocument": {
            "uri": ")" + uri + R"(",
            "languageId": ")" + languageId + R"(",
            "version": 1,
            "text": ")" + escaped + R"("
        }
    })";

    std::string msg = R"({"jsonrpc":"2.0","method":"textDocument/didOpen","params":)" + params + R"(})";
    SendMessage(msg);
}

void LspClient::ChangeDocument(const std::string& uri, const std::string& text) {
    std::string escaped;
    for (char c : text) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }

    static int version = 1;
    std::string params = R"({
        "textDocument": {
            "uri": ")" + uri + R"(",
            "version": )" + std::to_string(++version) + R"(
        },
        "contentChanges": [
            {"text": ")" + escaped + R"("}
        ]
    })";

    std::string msg = R"({"jsonrpc":"2.0","method":"textDocument/didChange","params":)" + params + R"(})";
    SendMessage(msg);
}

void LspClient::CloseDocument(const std::string& uri) {
    std::string params = R"({"textDocument":{"uri":")" + uri + R"("}})";
    std::string msg = R"({"jsonrpc":"2.0","method":"textDocument/didClose","params":)" + params + R"(})";
    SendMessage(msg);
}

void LspClient::RequestCompletions(const std::string& uri, int line, int column) {
    std::string params = R"({
        "textDocument": {"uri": ")" + uri + R"("},
        "position": {"line": )" + std::to_string(line) + R"(, "character": )" + std::to_string(column) + R"(}
    })";

    SendRequest("textDocument/completion", params);
}

std::string LspClient::SendRequest(const std::string& method, const std::string& params) {
    int id = m_nextId++;
    std::string msg = R"({"jsonrpc":"2.0","id":)" + std::to_string(id) + R"(,"method":")" + method + R"(","params":)" + params + R"(})";
    SendMessage(msg);
    return std::to_string(id);
}

void LspClient::ReaderThread() {
    while (m_running) {
        struct pollfd pfd;
        pfd.fd = m_pipeStdout;
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, 100); // 100ms timeout

        if (ret < 0) break;
        if (ret == 0) continue;

        char buf[4096];
        ssize_t n = read(m_pipeStdout, buf, sizeof(buf) - 1);
        if (n <= 0) break;

        buf[n] = 0;
        m_readBuffer += buf;

        // Process complete messages
        while (true) {
            // Look for Content-Length header
            auto pos = m_readBuffer.find("Content-Length: ");
            if (pos == std::string::npos) break;

            auto endLine = m_readBuffer.find("\r\n", pos);
            if (endLine == std::string::npos) break;

            std::string lenStr = m_readBuffer.substr(pos + 16, endLine - pos - 16);
            int contentLen = std::stoi(lenStr);

            // Find double CRLF
            auto headerEnd = m_readBuffer.find("\r\n\r\n", endLine);
            if (headerEnd == std::string::npos) break;

            int bodyStart = headerEnd + 4;
            if ((int)m_readBuffer.size() < bodyStart + contentLen) break;

            std::string body = m_readBuffer.substr(bodyStart, contentLen);
            m_readBuffer.erase(0, bodyStart + contentLen);

            ProcessMessage(body);
        }
    }
}

void LspClient::ProcessMessage(const std::string& json) {
    // Parse method from response
    auto methodPos = json.find("\"method\"");
    if (methodPos != std::string::npos) {
        // It's a notification
        auto methodStart = json.find('"', methodPos + 9);
        if (methodStart != std::string::npos) {
            auto methodEnd = json.find('"', methodStart + 1);
            if (methodEnd != std::string::npos) {
                std::string method = json.substr(methodStart + 1, methodEnd - methodStart - 1);

                if (method == "textDocument/publishDiagnostics") {
                    // Parse diagnostics
                    auto uriStart = json.find("\"uri\"");
                    if (uriStart == std::string::npos) return;
                    auto uriValStart = json.find('"', uriStart + 6);
                    if (uriValStart == std::string::npos) return;
                    auto uriValEnd = json.find('"', uriValStart + 1);
                    if (uriValEnd == std::string::npos) return;
                    std::string uri = json.substr(uriValStart + 1, uriValEnd - uriValStart - 1);

                    std::vector<LspDiagnostic> diags;

                    // Parse diagnostics array
                    auto diagArrayStart = json.find("\"diagnostics\"");
                    if (diagArrayStart != std::string::npos) {
                        auto arrStart = json.find('[', diagArrayStart);
                        if (arrStart != std::string::npos) {
                            size_t pos = arrStart + 1;
                            int depth = 1;
                            while (pos < json.size() && depth > 0) {
                                if (json[pos] == '{') depth++;
                                if (json[pos] == '}') depth--;
                                pos++;
                            }
                            // Simplified: count basic diagnostics
                            int count = 0;
                            pos = arrStart;
                            while ((pos = json.find("\"message\"", pos)) != std::string::npos && pos < arrStart + 20000) {
                                count++;
                                pos++;
                            }
                            if (count > 0)
                                EditorLog("LSP: " + std::to_string(count) + " diagnostic(s) for " + uri, count > 0 ? 1 : 0);
                        }
                    }

                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_diagnostics[uri] = diags;
                    }

                    if (m_diagnosticsCallback)
                        m_diagnosticsCallback(uri, diags);
                }
            }
        }
    }

    // Handle completion results
    auto resultPos = json.find("\"result\"");
    if (resultPos != std::string::npos) {
        auto itemsPos = json.find("\"items\"");
        if (itemsPos != std::string::npos) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_completions.clear();

            // Parse completion items (simplified)
            size_t pos = itemsPos + 7;
            while (pos < json.size()) {
                auto labelPos = json.find("\"label\"", pos);
                if (labelPos == std::string::npos || labelPos > json.find(']', pos)) break;

                auto labelStart = json.find('"', labelPos + 7);
                if (labelStart == std::string::npos) break;
                auto labelEnd = json.find('"', labelStart + 1);
                if (labelEnd == std::string::npos) break;

                LspCompletionItem item;
                item.label = json.substr(labelStart + 1, labelEnd - labelStart - 1);

                // Try to get detail
                auto detailPos = json.find("\"detail\"", labelEnd);
                if (detailPos != std::string::npos && detailPos < json.find('}', labelEnd)) {
                    auto detailStart = json.find('"', detailPos + 8);
                    if (detailStart != std::string::npos) {
                        auto detailEnd = json.find('"', detailStart + 1);
                        if (detailEnd != std::string::npos)
                            item.detail = json.substr(detailStart + 1, detailEnd - detailStart - 1);
                    }
                }

                m_completions.push_back(item);
                pos = labelEnd + 1;
            }
        }
    }
}

std::vector<LspDiagnostic> LspClient::GetDiagnostics(const std::string& uri) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_diagnostics.find(uri);
    if (it != m_diagnostics.end()) return it->second;
    return {};
}

std::vector<LspCompletionItem> LspClient::GetCompletions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_completions;
}

} // namespace planet
