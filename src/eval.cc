#include "ast.hh"
#include "object.hh"

const Object null_obj(Object::Type::Null, std::monostate());
const Object true_obj(Object::Type::Bool, true);
const Object false_obj(Object::Type::Bool, false);

static Object eval_statements(std::vector<Statement>& statements,
                              std::shared_ptr<Environment> env);
static Object eval_statement(Statement& stmt, std::shared_ptr<Environment> env);
static Object eval_return(ReturnStatement& ret, std::shared_ptr<Environment> env);
static Object eval_expression(Expression& exp, std::shared_ptr<Environment> env);
static Object eval_prefix(PrefixExpression::Operator oper, Object& right);
static Object eval_bang(Object& right);
static Object eval_minus(Object& right);
static Object eval_infix(InfixExpression::Operator oper, Object& left,
                         Object& right);
static Object eval_integer_infix(InfixExpression::Operator oper, int64_t left,
                                 int64_t right);
static Object eval_if(IfExpression& ife, std::shared_ptr<Environment> env);
static Object eval_block(BlockStatement& bs, std::shared_ptr<Environment> env);
static Object eval_identifier(Identifier& ident, std::shared_ptr<Environment> env);
static Object eval_function(FunctionLiteral& fn, std::shared_ptr<Environment> env);
static inline Object eval_integer(IntegerLiteral& integer);
static inline Object eval_boolean(BooleanLiteral& boolean);
static inline Object native_bool_to_bool_obj(bool input);
std::vector<Object> eval_expressions(std::vector<Expression>& exps,
                                     std::shared_ptr<Environment> env);
static Object apply_function(Function& fn, std::vector<Object>& args);
static Environment extend_function_env(Function& fn, std::vector<Object>& args);
static Object unwrap_return(Object& obj);

static bool is_truthy(Object& obj);
static inline bool is_error(Object& obj);

template <typename... Args>
std::string string_format(const std::string& format, Args... args);

Object eval(Program& program, std::shared_ptr<Environment> env) {
    return eval_statements(program.statements, env);
}

static Object eval_statements(std::vector<Statement>& statements,
                              std::shared_ptr<Environment> env) {
    Object obj;
    for (auto& stmt : statements) {
        obj = eval_statement(stmt, env);
        if (obj.type == Object::Type::Return) {
            return *std::get<std::shared_ptr<Object>>(obj.value);
        }
        if (obj.type == Object::Type::Error) {
            return obj;
        }
    }
    return obj;
}

static Object eval_statement(Statement& stmt, std::shared_ptr<Environment> env) {
    switch (stmt.type) {
    case Statement::Type::Let: {
        LetStatement let = std::get<LetStatement>(stmt.data);
        Object val = eval_expression(let.value, env);
        if (is_error(val)) {
            return val;
        }
        env->set(std::move(*let.name.value), std::move(val));
    } break;
    case Statement::Type::Ret:
        return eval_return(std::get<ReturnStatement>(stmt.data), env);
    case Statement::Type::Expression:
        return eval_expression(std::get<ExpressionStatement>(stmt.data).exp,
                               env);
    default:
        break;
    }
    return null_obj;
}

static Object eval_return(ReturnStatement& ret, std::shared_ptr<Environment> env) {
    Object val = eval_expression(ret.value, env);
    return Object(Object::Type::Return, std::make_shared<Object>(val));
}

static Object eval_expression(Expression& exp, std::shared_ptr<Environment> env) {
    switch (exp.type) {
    case Expression::Type::Integer:
        return eval_integer(std::get<IntegerLiteral>(exp.data));
    case Expression::Type::Boolean:
        return eval_boolean(std::get<BooleanLiteral>(exp.data));
    case Expression::Type::Prefix: {
        PrefixExpression pe = std::get<PrefixExpression>(exp.data);
        Object right = eval_expression(*pe.right, env);
        if (is_error(right)) {
            return right;
        }
        return eval_prefix(pe.oper, right);
    };
    case Expression::Type::Infix: {
        InfixExpression infix = std::get<InfixExpression>(exp.data);
        Object left = eval_expression(*infix.left, env);
        if (is_error(left)) {
            return left;
        }
        Object right = eval_expression(*infix.right, env);
        if (is_error(right)) {
            return right;
        }
        return eval_infix(infix.oper, left, right);
    }
    case Expression::Type::If:
        return eval_if(std::get<IfExpression>(exp.data), env);
    case Expression::Type::Identifier:
        return eval_identifier(std::get<Identifier>(exp.data), env);
    case Expression::Type::Function:
        return eval_function(std::get<FunctionLiteral>(exp.data), env);
    case Expression::Type::Call: {
        CallExpression call = std::get<CallExpression>(exp.data);
        Object fn = eval_expression(*call.function, env);
        if (is_error(fn)) {
            return fn;
        }
        std::vector<Object> args = eval_expressions(call.arguments, env);
        if (args.size() == 1 && is_error(args[0])) {
            return args[0];
        }
        if (fn.type != Object::Type::Function) {
            return Object(
                Object::Type::Error,
                string_format("not a function: %s", fn.type_to_string()));
        }
        return apply_function(std::get<Function>(fn.value), args);
    }
    default:
        break;
    }
    return null_obj;
}

static Object eval_prefix(PrefixExpression::Operator oper, Object& right) {
    switch (oper) {
    case PrefixExpression::Operator::Bang:
        return eval_bang(right);
    case PrefixExpression::Operator::Minus:
        return eval_minus(right);
    default:
        break;
    }
    return Object(Object::Type::Error,
                  string_format("unknown operator: %s%s",
                                prefix_oper_to_string(oper),
                                right.type_to_string()));
}

static Object eval_bang(Object& right) {
    switch (right.type) {
    case Object::Type::Null:
        return true_obj;
    case Object::Type::Int:
        return false_obj;
    case Object::Type::Bool:
        if (std::get<bool>(right.value)) {
            return false_obj;
        }
        return true_obj;
    default:
        break;
    }
    return false_obj;
}

static Object eval_minus(Object& right) {
    if (right.type != Object::Type::Int) {
        return Object(
            Object::Type::Error,
            string_format("unknown operator: -%s", right.type_to_string()));
    }
    int64_t value = -(std::get<int64_t>(right.value));
    return Object(Object::Type::Int, value);
}

static Object eval_infix(InfixExpression::Operator oper, Object& left,
                         Object& right) {
    if (left.type != right.type) {
        return Object(
            Object::Type::Error,
            string_format("type mismatch: %s %s %s", left.type_to_string(),
                          infix_oper_to_string(oper), right.type_to_string()));
    }
    if (left.type == Object::Type::Int && right.type == Object::Type::Int) {
        int64_t lval = std::get<int64_t>(left.value);
        int64_t rval = std::get<int64_t>(right.value);
        return eval_integer_infix(oper, lval, rval);
    }
    if (oper == InfixExpression::Operator::Eq) {
        return native_bool_to_bool_obj(left == right);
    }
    if (oper == InfixExpression::Operator::NotEq) {
        return native_bool_to_bool_obj(left != right);
    }
    return Object(
        Object::Type::Error,
        string_format("unknown operator: %s %s %s", left.type_to_string(),
                      infix_oper_to_string(oper), right.type_to_string()));
}

static Object eval_integer_infix(InfixExpression::Operator oper, int64_t left,
                                 int64_t right) {
    switch (oper) {
    case InfixExpression::Operator::Plus:
        return Object(Object::Type::Int, left + right);
    case InfixExpression::Operator::Minus:
        return Object(Object::Type::Int, left - right);
    case InfixExpression::Operator::Asterisk:
        return Object(Object::Type::Int, left * right);
    case InfixExpression::Operator::Slash:
        return Object(Object::Type::Int, left / right);
    case InfixExpression::Operator::Lt:
        return native_bool_to_bool_obj(left < right);
    case InfixExpression::Operator::Gt:
        return native_bool_to_bool_obj(left > right);
    case InfixExpression::Operator::Eq:
        return native_bool_to_bool_obj(left == right);
    case InfixExpression::Operator::NotEq:
        return native_bool_to_bool_obj(left != right);
    }
    return null_obj;
}

static Object eval_if(IfExpression& ife, std::shared_ptr<Environment> env) {
    Object cond = eval_expression(*ife.condition, env);
    if (is_error(cond)) {
        return cond;
    }
    if (is_truthy(cond)) {
        return eval_block(ife.consequence, env);
    } else if (ife.alternative.has_value()) {
        return eval_block(*ife.alternative, env);
    } else {
        return null_obj;
    }
}

static Object eval_block(BlockStatement& bs, std::shared_ptr<Environment> env) {
    Object res;
    for (auto& stmt : bs.stmts) {
        res = eval_statement(stmt, env);
        if (res.type == Object::Type::Return) {
            return res;
        }
        if (res.type == Object::Type::Error) {
            return res;
        }
    }
    return res;
}

static Object eval_identifier(Identifier& ident, std::shared_ptr<Environment> env) {
    Object obj = env->get(*ident.value);
    if (obj.type == Object::Type::Null) {
        return Object(Object::Type::Error,
                      "identifier not found: " + *ident.value);
    }
    return obj;
}

static Object eval_function(FunctionLiteral& fn, std::shared_ptr<Environment> env) {
    Function func(std::move(fn.params), std::move(fn.body), env);
    return Object(Object::Type::Function, func);
}

std::vector<Object> eval_expressions(std::vector<Expression>& exps,
                                     std::shared_ptr<Environment> env) {
    std::vector<Object> res;
    for (auto& e : exps) {
        Object obj = eval_expression(e, env);
        if (is_error(obj)) {
            res.clear();
            res.push_back(obj);
            return res;
        }
        res.push_back(obj);
    }
    return res;
}

static Object apply_function(Function& fn, std::vector<Object>& args) {
    std::shared_ptr<Environment> extended_env = std::make_shared<Environment>(extend_function_env(fn, args));
    Object evaluated = eval_block(fn.body, extended_env);
    return unwrap_return(evaluated);
}

static Environment extend_function_env(Function& fn,
                                       std::vector<Object>& args) {
    Environment env = Environment(*fn.env);
    size_t i, len = fn.parameters.size();
    for (i = 0; i < len; ++i) {
        env.set(*fn.parameters[i].value, args[i]);
    }
    return env;
}

static Object unwrap_return(Object& obj) {
    if (obj.type == Object::Type::Return) {
        return *std::get<std::shared_ptr<Object>>(obj.value);
    }
    return obj;
}

static inline Object eval_integer(IntegerLiteral& integer) {
    return Object(Object::Type::Int, integer.value);
}

static inline Object eval_boolean(BooleanLiteral& boolean) {
    return native_bool_to_bool_obj(boolean.value);
}

static inline Object native_bool_to_bool_obj(bool input) {
    if (input) {
        return true_obj;
    }
    return false_obj;
}

static bool is_truthy(Object& obj) {
    switch (obj.type) {
    case Object::Type::Null:
        return false;
    case Object::Type::Int:
        return true;
    case Object::Type::Bool:
        return std::get<bool>(obj.value);
    default:
        break;
    }
    return false;
}

static inline bool is_error(Object& obj) {
    return obj.type == Object::Type::Error;
}

template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) +
                 1; // Extra space for '\0'
    if (size_s <= 0) {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(),
                       buf.get() + size - 1); // We don't want the '\0' inside
}
