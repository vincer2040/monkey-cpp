#pragma once

#include "ast.hh"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Function {
    std::vector<Identifier> parameters;
    BlockStatement body;
    std::shared_ptr<struct Environment> env;
    Function(std::vector<Identifier> parameters, BlockStatement body,
             std::shared_ptr<struct Environment> env);
};

typedef std::variant<std::monostate, int64_t, bool, std::string,
                     std::shared_ptr<struct Object>, struct Function>
    ObjectValue;

struct Object {
    enum class Type {
        Null,
        Int,
        Bool,
        Error,
        Return,
        Function,
    } type;
    ObjectValue value;
    Object();
    Object(Object::Type type, ObjectValue value);
    std::string inspect();
    const char* type_to_string();
    bool operator==(Object& right);
    bool operator!=(Object& right);
};

struct Environment {
    std::unordered_map<std::string, Object> store;
    std::optional<std::shared_ptr<struct Environment>> outer;
    Environment();
    Environment(std::shared_ptr<struct Environment> outer);
    Object& get(std::string& name);
    void set(std::string name, Object value);
};
