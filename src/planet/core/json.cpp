#include "planet/core/json.h"
#include <cctype>
#include <stdexcept>

namespace planet {
namespace json {

const Value Value::s_null;

const Value& Value::operator[](size_t i) const {
    if (type != Type::Array || i >= arr.size()) return s_null;
    return arr[i];
}

const Value& Value::operator[](const std::string& k) const {
    if (type != Type::Object) return s_null;
    auto it = obj.find(k);
    if (it == obj.end()) return s_null;
    return it->second;
}

size_t Value::size() const {
    if (type == Type::Array) return arr.size();
    if (type == Type::Object) return obj.size();
    return 0;
}

bool Value::has(const std::string& k) const {
    return type == Type::Object && obj.find(k) != obj.end();
}

class Parser {
public:
    Parser(const std::string& input) : m_input(input), m_pos(0) {}

    Value parseValue() {
        skipWhitespace();
        if (m_pos >= m_input.size()) return Value{};

        char c = m_input[m_pos];
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();

        return Value{};
    }

private:
    void skipWhitespace() {
        while (m_pos < m_input.size()) {
            char c = m_input[m_pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                m_pos++;
            else
                break;
        }
    }

    char peek() {
        skipWhitespace();
        return m_pos < m_input.size() ? m_input[m_pos] : '\0';
    }

    char consume() {
        return m_pos < m_input.size() ? m_input[m_pos++] : '\0';
    }

    void expect(char c) {
        char got = consume();
        if (got != c) {
            throw std::runtime_error(std::string("expected '") + c + "' got '" + got + "'");
        }
    }

    Value parseObject() {
        Value v;
        v.type = Type::Object;
        expect('{');
        if (peek() == '}') { consume(); return v; }

        while (true) {
            skipWhitespace();
            auto key = parseString();
            skipWhitespace();
            expect(':');
            auto val = parseValue();
            v.obj[key.asString()] = val;
            skipWhitespace();
            if (peek() == '}') { consume(); return v; }
            if (peek() == ',') { consume(); continue; }
            throw std::runtime_error("expected ',' or '}' in object");
        }
    }

    Value parseArray() {
        Value v;
        v.type = Type::Array;
        expect('[');
        if (peek() == ']') { consume(); return v; }

        while (true) {
            v.arr.push_back(parseValue());
            skipWhitespace();
            if (peek() == ']') { consume(); return v; }
            if (peek() == ',') { consume(); continue; }
            throw std::runtime_error("expected ',' or ']' in array");
        }
    }

    Value parseString() {
        Value v;
        v.type = Type::String;
        expect('"');
        std::string s;
        while (m_pos < m_input.size()) {
            char c = m_input[m_pos];
            if (c == '"') { m_pos++; break; }
            if (c == '\\') {
                m_pos++;
                if (m_pos >= m_input.size()) break;
                char next = m_input[m_pos];
                switch (next) {
                    case '"': s += '"'; break;
                    case '\\': s += '\\'; break;
                    case '/': s += '/'; break;
                    case 'n': s += '\n'; break;
                    case 't': s += '\t'; break;
                    case 'r': s += '\r'; break;
                    default: s += next; break;
                }
                m_pos++;
            } else {
                s += c;
                m_pos++;
            }
        }
        v.strVal = s;
        return v;
    }

    Value parseNumber() {
        Value v;
        v.type = Type::Number;
        size_t start = m_pos;
        if (m_input[m_pos] == '-') m_pos++;
        while (m_pos < m_input.size() && std::isdigit(m_input[m_pos])) m_pos++;
        if (m_pos < m_input.size() && m_input[m_pos] == '.') {
            m_pos++;
            while (m_pos < m_input.size() && std::isdigit(m_input[m_pos])) m_pos++;
        }
        if (m_pos < m_input.size() && (m_input[m_pos] == 'e' || m_input[m_pos] == 'E')) {
            m_pos++;
            if (m_pos < m_input.size() && (m_input[m_pos] == '+' || m_input[m_pos] == '-')) m_pos++;
            while (m_pos < m_input.size() && std::isdigit(m_input[m_pos])) m_pos++;
        }
        v.numVal = std::stod(m_input.substr(start, m_pos - start));
        return v;
    }

    Value parseBool() {
        Value v;
        v.type = Type::Bool;
        if (m_input.substr(m_pos, 4) == "true") {
            v.boolVal = true;
            m_pos += 4;
        } else if (m_input.substr(m_pos, 5) == "false") {
            v.boolVal = false;
            m_pos += 5;
        }
        return v;
    }

    Value parseNull() {
        Value v;
        if (m_input.substr(m_pos, 4) == "null") {
            m_pos += 4;
        }
        return v;
    }

    const std::string& m_input;
    size_t m_pos;
};

Value parse(const std::string& input) {
    Parser parser(input);
    Value v = parser.parseValue();
    return v;
}

} // namespace json
} // namespace planet
