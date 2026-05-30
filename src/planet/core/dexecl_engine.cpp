#include "planet/core/dexecl_engine.h"
#include "planet/core/console.h"
#include "planet/core/engine.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include "planet/core/logger.h"

namespace planet {

DexeclEngine& DexeclEngine::Instance() {
    static DexeclEngine instance;
    return instance;
}

void DexeclEngine::Clear() {
    m_variables.clear();
    m_lines.clear();
    m_loopStack.clear();
    m_pc = 0;
}

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool DexeclEngine::ParseCondition(const std::string& expr, std::string& left, std::string& op, std::string& right) {
    std::string e = trim(expr);
    if (e.empty()) return false;

    const char* ops[] = {">=", "<=", "!=", "==", ">", "<"};
    for (const char* o : ops) {
        size_t pos = e.find(o);
        if (pos != std::string::npos) {
            left  = trim(e.substr(0, pos));
            op    = o;
            right = trim(e.substr(pos + std::strlen(o)));
            return true;
        }
    }
    return false;
}

bool DexeclEngine::EvaluateCondition(const std::string& expr) {
    std::string left, op, right;
    if (!ParseCondition(expr, left, op, right)) return false;

    auto resolve = [&](const std::string& s) -> float {
        char* end = nullptr;
        float v = std::strtof(s.c_str(), &end);
        if (end == s.c_str()) {
            return HasVariable(s) ? GetVariable(s) : 0.0f;
        }
        return v;
    };

    float lVal = resolve(left);
    float rVal = resolve(right);

    if (op == "==") return lVal == rVal;
    if (op == "!=") return lVal != rVal;
    if (op == ">=") return lVal >= rVal;
    if (op == "<=") return lVal <= rVal;
    if (op == ">")  return lVal > rVal;
    if (op == "<")  return lVal < rVal;
    return false;
}

void DexeclEngine::SetVariable(const std::string& name, float value) {
    m_variables[name] = value;
}

float DexeclEngine::GetVariable(const std::string& name, float defaultVal) const {
    auto it = m_variables.find(name);
    return (it != m_variables.end()) ? it->second : defaultVal;
}

void DexeclEngine::DeleteVariable(const std::string& name) {
    m_variables.erase(name);
}

bool DexeclEngine::HasVariable(const std::string& name) const {
    return m_variables.find(name) != m_variables.end();
}

std::string DexeclEngine::ExpandVariables(const std::string& line) {
    std::string result;
    result.reserve(line.size() * 2);
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '$' && i + 1 < line.size()) {
            size_t j = i + 1;
            while (j < line.size() && (std::isalnum(static_cast<unsigned char>(line[j])) || line[j] == '_')) j++;
            std::string varName = line.substr(i + 1, j - i - 1);
            float val = GetVariable(varName, 0.0f);
            std::ostringstream oss;
            oss << val;
            result += oss.str();
            i = j - 1;
        } else {
            result += line[i];
        }
    }
    return result;
}

void DexeclEngine::ExecuteLine(const std::string& rawLine) {
    std::string line = trim(rawLine);
    if (line.empty() || (line.size() > 1 && line[0] == '/' && line[1] == '/')) return;

    line = ExpandVariables(line);
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "set") {
        std::string name;
        iss >> name;
        if (name.empty()) return;
        std::string rest;
        std::getline(iss, rest);
        rest = trim(rest);
        float value = 0.0f;
        if (!rest.empty()) {
            // Try arithmetic: "a + b" or "a - b"
            size_t plus = rest.find('+');
            size_t minus = rest.find('-');
            if (plus != std::string::npos && (minus == std::string::npos || plus < minus)) {
                std::string l = trim(rest.substr(0, plus));
                std::string r = trim(rest.substr(plus + 1));
                float lv = HasVariable(l) ? GetVariable(l) : static_cast<float>(std::atof(l.c_str()));
                float rv = HasVariable(r) ? GetVariable(r) : static_cast<float>(std::atof(r.c_str()));
                value = lv + rv;
            } else if (minus != std::string::npos) {
                std::string l = trim(rest.substr(0, minus));
                std::string r = trim(rest.substr(minus + 1));
                float lv = HasVariable(l) ? GetVariable(l) : static_cast<float>(std::atof(l.c_str()));
                float rv = HasVariable(r) ? GetVariable(r) : static_cast<float>(std::atof(r.c_str()));
                value = lv - rv;
            } else if (HasVariable(rest)) {
                value = GetVariable(rest);
            } else {
                value = static_cast<float>(std::atof(rest.c_str()));
            }
        }
        SetVariable(name, value);
        return;
    }

    if (cmd == "echo" || cmd == "print") {
        std::string msg;
        std::getline(iss, msg);
        Console::Instance().Print(trim(msg));
        return;
    }

    if (cmd == "unset" || cmd == "delete") {
        std::string name;
        iss >> name;
        DeleteVariable(name);
        return;
    }

    if (cmd == "inc" || cmd == "increment") {
        std::string name;
        float amt = 1.0f;
        iss >> name;
        std::string rest;
        iss >> rest;
        if (!rest.empty()) amt = static_cast<float>(std::atof(rest.c_str()));
        SetVariable(name, GetVariable(name) + amt);
        return;
    }

    Console::Instance().Execute(line);
}

static size_t findMatchingEnd(const std::vector<std::string>& lines, size_t start,
                              const std::string& startKw, const std::string& endKw) {
    int depth = 1;
    size_t i = start + 1;
    while (i < lines.size() && depth > 0) {
        std::string l = trim(lines[i]);
        std::istringstream iss(l);
        std::string first;
        iss >> first;
        if (first == startKw) depth++;
        if (first == endKw)   depth--;
        if (depth > 0) i++;
    }
    return (depth == 0) ? i : lines.size();
}

void DexeclEngine::ExecuteString(const std::string& code) {
    m_lines.clear();
    std::istringstream stream(code);
    std::string line;
    while (std::getline(stream, line)) {
        m_lines.push_back(line);
    }

    m_pc = 0;
    m_loopStack.clear();

    while (m_pc < m_lines.size()) {
        std::string raw = m_lines[m_pc];
        std::string l = trim(raw);
        if (l.empty() || (l.size() > 1 && l[0] == '/' && l[1] == '/')) {
            m_pc++;
            continue;
        }

        std::istringstream iss(l);
        std::string cmd;
        iss >> cmd;

        // ---- IF ----
        if (cmd == "if") {
            std::string cond;
            std::getline(iss, cond);
            cond = trim(cond);
            size_t endifPos = findMatchingEnd(m_lines, m_pc, "if", "endif");

            bool result = EvaluateCondition(cond);

            if (result) {
                m_pc++;
                continue;
            } else {
                // Find else at same depth
                int depth = 1;
                size_t elsePos = std::string::npos;
                for (size_t i = m_pc + 1; i < endifPos; i++) {
                    std::string sub = trim(m_lines[i]);
                    std::istringstream siss(sub);
                    std::string first;
                    siss >> first;
                    if (first == "if") depth++;
                    if (first == "endif") depth--;
                    if (first == "else" && depth == 1) {
                        elsePos = i;
                        break;
                    }
                }
                m_pc = (elsePos != std::string::npos) ? elsePos + 1 : endifPos + 1;
            }
            continue;
        }

        if (cmd == "else") {
            int depth = 1;
            size_t i = m_pc + 1;
            while (i < m_lines.size() && depth > 0) {
                std::string sub = trim(m_lines[i]);
                std::istringstream siss(sub);
                std::string first;
                siss >> first;
                if (first == "if") depth++;
                if (first == "endif") depth--;
                i++;
            }
            m_pc = i;
            continue;
        }

        if (cmd == "endif") {
            m_pc++;
            continue;
        }

        // ---- FOR ----
        if (cmd == "for") {
            std::string varName, startStr, endStr;
            iss >> varName >> startStr >> endStr;

            auto resolve = [&](const std::string& s) -> float {
                if (HasVariable(s)) return GetVariable(s);
                return static_cast<float>(std::atof(s.c_str()));
            };

            float start = resolve(startStr);
            float end   = resolve(endStr);
            float step  = (start <= end) ? 1.0f : -1.0f;

            size_t endForPos = findMatchingEnd(m_lines, m_pc, "for", "endfor");

            LoopFrame lf;
            lf.type      = FlowState::ForLoop;
            lf.varName   = varName;
            lf.start     = start;
            lf.end       = end;
            lf.step      = step;
            lf.loopStart = m_pc;         // 'for' line
            lf.loopEnd   = endForPos;    // 'endfor' line
            lf.bodyEnd   = m_pc + 1;     // first body line

            SetVariable(varName, start);
            m_loopStack.push_back(lf);
            m_pc = m_pc + 1;
            continue;
        }

        if (cmd == "endfor") {
            if (!m_loopStack.empty() && m_loopStack.back().type == FlowState::ForLoop) {
                LoopFrame& lf = m_loopStack.back();
                float cur = GetVariable(lf.varName) + lf.step;
                SetVariable(lf.varName, cur);

                bool cont = (lf.step > 0) ? (cur <= lf.end) : (cur >= lf.end);
                if (cont) {
                    m_pc = lf.loopStart + 1;
                } else {
                    m_loopStack.pop_back();
                    m_pc = lf.loopEnd + 1;
                }
            } else {
                m_pc++;
            }
            continue;
        }

        // ---- WHILE ----
        if (cmd == "while") {
            std::string cond;
            std::getline(iss, cond);
            cond = trim(cond);

            size_t endWhilePos = findMatchingEnd(m_lines, m_pc, "while", "endwhile");

            if (!EvaluateCondition(cond)) {
                m_pc = endWhilePos + 1;
                continue;
            }

            LoopFrame lf;
            lf.type      = FlowState::WhileLoop;
            lf.loopStart = m_pc;
            lf.loopEnd   = endWhilePos;
            lf.bodyEnd   = m_pc + 1;
            m_loopStack.push_back(lf);
            m_pc = m_pc + 1;
            continue;
        }

        if (cmd == "endwhile") {
            if (!m_loopStack.empty() && m_loopStack.back().type == FlowState::WhileLoop) {
                LoopFrame& lf = m_loopStack.back();
                std::string whileLine = trim(m_lines[lf.loopStart]);
                std::string wcond;
                {
                    std::istringstream wiss(whileLine);
                    std::string w;
                    wiss >> w; // skip "while"
                    std::getline(wiss, wcond);
                    wcond = trim(wcond);
                }

                if (EvaluateCondition(wcond)) {
                    m_pc = lf.loopStart + 1;
                } else {
                    m_loopStack.pop_back();
                    m_pc = lf.loopEnd + 1;
                }
            } else {
                m_pc++;
            }
            continue;
        }

        // Ordinary command
        ExecuteLine(l);
        m_pc++;
    }
}

bool DexeclEngine::ExecuteFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Console::Instance().PrintError("DEXECL: Cannot open file: " + filename);
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    Console::Instance().Print("DEXECL: Executing " + filename);
    ExecuteString(buffer.str());
    return true;
}

} // namespace planet
