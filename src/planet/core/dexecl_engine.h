#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace planet {

class DexeclEngine {
public:
    static DexeclEngine& Instance();

    void Clear();
    bool ExecuteFile(const std::string& filename);
    void ExecuteString(const std::string& code);
    void ExecuteLine(const std::string& line);

    void SetVariable(const std::string& name, float value);
    float GetVariable(const std::string& name, float defaultVal = 0.0f) const;
    void DeleteVariable(const std::string& name);
    bool HasVariable(const std::string& name) const;

    const std::unordered_map<std::string, float>& GetVariables() const { return m_variables; }

private:
    DexeclEngine() = default;
    DexeclEngine(const DexeclEngine&) = delete;
    DexeclEngine& operator=(const DexeclEngine&) = delete;

    enum class FlowState {
        Normal,
        IfSkip,
        ForLoop,
        WhileLoop,
    };

    struct LoopFrame {
        FlowState type;
        std::string varName;
        float start, end, step;
        size_t loopStart;
        size_t loopEnd;
        size_t bodyEnd;
    };

    bool EvaluateCondition(const std::string& expr);
    bool ParseCondition(const std::string& expr, std::string& left, std::string& op, std::string& right);

    std::string ExpandVariables(const std::string& line);

    std::unordered_map<std::string, float> m_variables;
    std::vector<std::string> m_lines;
    std::vector<LoopFrame> m_loopStack;
    size_t m_pc = 0;
};

} // namespace planet
