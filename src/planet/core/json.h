#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace planet {
namespace json {

enum class Type { Null, Bool, Number, String, Array, Object };

class Value {
public:
    Type type = Type::Null;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::vector<Value> arr;
    std::unordered_map<std::string, Value> obj;

    bool isNull() const { return type == Type::Null; }
    bool isBool() const { return type == Type::Bool; }
    bool isNumber() const { return type == Type::Number; }
    bool isString() const { return type == Type::String; }
    bool isArray() const { return type == Type::Array; }
    bool isObject() const { return type == Type::Object; }

    bool asBool() const { return boolVal; }
    double asNumber() const { return numVal; }
    int asInt() const { return static_cast<int>(numVal); }
    float asFloat() const { return static_cast<float>(numVal); }
    const std::string& asString() const { return strVal; }

    const Value& operator[](size_t i) const;
    const Value& operator[](const std::string& k) const;
    size_t size() const;
    bool has(const std::string& k) const;

private:
    static const Value s_null;
};

Value parse(const std::string& input);

} // namespace json
} // namespace planet
