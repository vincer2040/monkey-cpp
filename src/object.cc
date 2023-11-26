#include "object.hh"
#include "util.hh"

Object::Object() : type(Object::Type::Null), value(std::monostate()) {}

Object::Object(Object::Type type, ObjectValue value)
    : type(type), value(std::move(value)) {}

Function::Function(std::vector<Identifier> parameters, BlockStatement body,
                   std::shared_ptr<Environment> env)
    : parameters(std::move(parameters)), body(std::move(body)), env(env) {}

std::string Object::inspect() {
    switch (type) {
    case Type::Null:
        return "Null";
    case Type::Int:
        return std::to_string(std::get<int64_t>(value));
    case Type::Bool:
        if (std::get<bool>(value)) {
            return "true";
        }
        return "false";
    case Type::Return:
        return std::get<std::shared_ptr<Object>>(value)->inspect();
    case Type::Error:
        return "Error: " + std::get<std::string>(value);
    case Type::Function: {
        std::string res;
        auto fn = std::get<Function>(value);
        size_t i, len = fn.parameters.size();
        res.append("fn(");
        for (i = 0; i < len; ++i) {
            res.append(fn.parameters[i].string());
            if (i != len - 1) {
                res.append(", ");
            }
        }
        res.append(") {\n");
        res.append(fn.body.string());
        res.append("\n}");
        return res;
    }
    }
    unreachable;
    return "";
}

const char* Object::type_to_string() {
    switch (type) {
    case Type::Null:
        return "NULL";
    case Type::Int:
        return "INTEGER";
    case Type::Bool:
        return "BOOLEAN";
    case Type::Error:
        return "ERROR";
    case Type::Return:
        return "RETURN";
    case Type::Function:
        return "FUNCTION";
    }
    return "";
}

bool Object::operator==(Object& right) {
    if (type != right.type) {
        return false;
    }
    switch (type) {
    case Type::Null:
        return true;
    case Type::Int:
        return std::get<int64_t>(value) == std::get<int64_t>(right.value);
    case Type::Bool:
        return std::get<bool>(value) == std::get<bool>(right.value);
    case Type::Return:
        return false;
    case Type::Error:
        return std::get<std::string>(value) ==
               std::get<std::string>(right.value);
    case Type::Function:
        return false;
    }
    return false;
}

bool Object::operator!=(Object& right) {
    if (type != right.type) {
        return true;
    }
    switch (type) {
    case Type::Null:
        return false;
    case Type::Int:
        return std::get<int64_t>(value) != std::get<int64_t>(right.value);
    case Type::Bool:
        return std::get<bool>(value) != std::get<bool>(right.value);
    case Type::Return:
        return false;
    case Type::Error:
        return std::get<std::string>(value) !=
               std::get<std::string>(right.value);
    case Type::Function:
        return false;
    }
    return true;
}

Environment::Environment()
    : store(std::unordered_map<std::string, Object>()), outer({}) {}

Environment::Environment(std::shared_ptr<Environment> outer)
    : store(std::unordered_map<std::string, Object>()), outer(outer) {}

Object& Environment::get(std::string& name) {
    Object& obj = store[name];
    if (obj.type == Object::Type::Null) {
        if (outer.has_value()) {
            return (**outer).get(name);
        }
    }
    return obj;
}

void Environment::set(std::string name, Object value) {
    store[std::move(name)] = std::move(value);
}
