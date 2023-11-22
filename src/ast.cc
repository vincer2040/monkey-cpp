#include "ast.hh"
#include "token.hh"
#include "util.hh"

Statement::Statement() : type(Statement::Type::Inv), data(std::monostate()) {}

LetStatement::LetStatement(Token tok, Identifier name, Expression& value)
    : tok(tok), name(name), value(value) {}

ReturnStatement::ReturnStatement(Token tok, Expression& value)
    : tok(tok), value(value) {}

ExpressionStatement::ExpressionStatement(Token tok, Expression e)
    : tok(tok), exp(e) {}

Expression::Expression() : type(Expression::Type::Inv) {}

Expression::Expression(Expression::Type type, ExpressionVariant data)
    : type(type), data(data) {}

Identifier::Identifier(Token tok, std::shared_ptr<std::string> value)
    : tok(tok), value(value) {}

IntegerLiteral::IntegerLiteral(Token tok, int64_t value)
    : tok(tok), value(value) {}

BooleanLiteral::BooleanLiteral(Token tok, bool value)
    : tok(tok), value(value) {}

PrefixExpression::PrefixExpression(Token tok, PrefixExpression::Operator oper,
                                   Expression& right)
    : tok(tok), oper(oper), right(std::make_shared<Expression>(right)) {}

InfixExpression::InfixExpression(Token tok, InfixExpression::Operator oper,
                                 Expression& left, Expression& right)
    : tok(tok), oper(oper), left(std::make_shared<Expression>(left)),
      right(std::make_shared<Expression>(right)) {}

BlockStatement::BlockStatement()
    : tok(Token()), stmts(std::vector<Statement>()) {}

BlockStatement::BlockStatement(Token tok, std::vector<Statement>& stmts)
    : tok(tok), stmts(stmts) {}

IfExpression::IfExpression(Token tok, Expression& condition,
                           BlockStatement& consequence,
                           std::optional<BlockStatement>& alternative)
    : tok(tok), condition(std::make_shared<Expression>(condition)),
      consequence(consequence), alternative(alternative) {}

FunctionLiteral::FunctionLiteral(Token tok, std::vector<Identifier>& params,
                                 BlockStatement& body)
    : tok(tok), params(params), body(body) {}

CallExpression::CallExpression(Token tok, Expression& function,
                               std::vector<Expression>& arguments)
    : tok(tok), function(std::make_shared<Expression>(function)),
      arguments(arguments) {}

const char* Program::token_literal() {
    if (statements.size() < 0) {
        return nullptr;
    }
    return statements[0].token_literal();
}

const char* Statement::token_literal() {
    switch (type) {
    case Type::Let:
        return std::get<LetStatement>(data).token_literal();
    case Type::Ret:
        return std::get<ReturnStatement>(data).token_literal();
    case Type::Expression:
        return std::get<ExpressionStatement>(data).token_literal();
    default:
        break;
    }
    unreachable;
    return "";
}

const char* LetStatement::token_literal() { return tok.get_literal(); }

const char* ReturnStatement::token_literal() { return tok.get_literal(); }

const char* Expression::token_literal() {
    switch (type) {
    case Type::Identifier:
        return std::get<Identifier>(data).token_literal();
    case Type::Integer:
        return std::get<IntegerLiteral>(data).token_literal();
    case Type::Boolean:
        return std::get<BooleanLiteral>(data).token_literal();
    case Type::Prefix:
        return std::get<PrefixExpression>(data).token_literal();
    case Type::Infix:
        return std::get<InfixExpression>(data).token_literal();
    case Type::If:
        return std::get<IfExpression>(data).token_literal();
    case Type::Function:
        return std::get<FunctionLiteral>(data).token_literal();
    case Type::Call:
        return std::get<CallExpression>(data).token_literal();
    default:
        return "";
    }
    return "";
}

const char* ExpressionStatement::token_literal() { return tok.get_literal(); }

const char* Identifier::token_literal() { return tok.get_literal(); }

const char* IntegerLiteral::token_literal() { return tok.get_literal(); }

const char* BooleanLiteral::token_literal() { return tok.get_literal(); }

const char* PrefixExpression::token_literal() { return tok.get_literal(); }

const char* InfixExpression::token_literal() { return tok.get_literal(); }

const char* BlockStatement::token_literal() { return tok.get_literal(); }

const char* IfExpression::token_literal() { return tok.get_literal(); }

const char* FunctionLiteral::token_literal() { return tok.get_literal(); }

const char* CallExpression::token_literal() { return tok.get_literal(); }

std::string Program::string() {
    std::string res;
    for (auto& stmt : statements) {
        res.append(stmt.string());
    }
    return res;
}

std::string Statement::string() {
    switch (type) {
    case Type::Let:
        return std::get<LetStatement>(data).string();
    case Type::Ret:
        return std::get<ReturnStatement>(data).string();
    case Type::Expression:
        return std::get<ExpressionStatement>(data).string();
    default:
        break;
    }
    unreachable;
    return "";
}

std::string LetStatement::string() {
    std::string res;
    res.append(token_literal());
    res.append(name.string());
    res.append(" = ");
    if (value.type != Expression::Type::Inv) {
        res.append(value.string());
    }
    res.push_back(';');
    return res;
}

std::string ReturnStatement::string() {
    std::string res;
    res.append(token_literal());
    res.append(" ");
    if (value.type != Expression::Type::Inv) {
        res.append(value.string());
    }
    res.push_back(';');
    return res;
}

std::string ExpressionStatement::string() {
    if (exp.type == Expression::Type::Inv) {
        return "";
    }
    return exp.string();
}

std::string Expression::string() {
    switch (type) {
    case Type::Identifier:
        return std::get<Identifier>(data).string();
    case Type::Integer:
        return std::get<IntegerLiteral>(data).string();
    case Type::Boolean:
        return std::get<BooleanLiteral>(data).string();
    case Type::Prefix:
        return std::get<PrefixExpression>(data).string();
    case Type::Infix:
        return std::get<InfixExpression>(data).string();
    case Type::If:
        return std::get<IfExpression>(data).string();
    case Type::Function:
        return std::get<FunctionLiteral>(data).string();
    case Type::Call:
        return std::get<CallExpression>(data).string();
    default:
        return "";
    }
}

std::string Identifier::string() { return *value; }

std::string IntegerLiteral::string() { return token_literal(); }

std::string BooleanLiteral::string() { return token_literal(); }

std::string PrefixExpression::string() {
    std::string res;
    res.push_back('(');
    res.append(prefix_oper_to_string(oper));
    res.append(right->string());
    res.push_back(')');
    return res;
}

std::string InfixExpression::string() {
    std::string res;
    res.push_back('(');
    res.append(left->string());
    res.push_back(' ');
    res.append(infix_oper_to_string(oper));
    res.push_back(' ');
    res.append(right->string());
    res.push_back(')');
    return res;
}

std::string BlockStatement::string() {
    std::string res;
    for (auto& stmt : stmts) {
        res.append(stmt.string());
    }
    return res;
}

std::string IfExpression::string() {
    std::string res;
    res.append("if");
    res.append(condition->string());
    res.push_back(' ');
    res.append(consequence.string());
    if (alternative.has_value()) {
        res.append("else ");
        res.append(alternative->string());
    }
    return res;
}

std::string FunctionLiteral::string() {
    std::string res;
    size_t i, len = params.size();
    res.append(token_literal());
    res.push_back(')');
    for (i = 0; i < len; ++i) {
        res.append(params[i].string());
        if (i != len - 1) {
            res.append(", ");
        }
    }
    res.push_back(')');
    res.append(body.string());
    return res;
}

std::string CallExpression::string() {
    std::string res;
    size_t i, len = arguments.size();
    res.append(function->string());
    res.push_back('(');
    for (i = 0; i < len; ++i) {
        res.append(arguments[i].string());
        if (i != len - 1) {
            res.append(", ");
        }
    }
    res.push_back(')');
    return res;
}

const char* prefix_oper_to_string(PrefixExpression::Operator oper) {
    switch (oper) {
    case PrefixExpression::Operator::Bang:
        return "!";
    case PrefixExpression::Operator::Minus:
        return "-";
    }
    unreachable;
    return "";
}

const char* infix_oper_to_string(InfixExpression::Operator oper) {
    switch (oper) {
    case InfixExpression::Operator::Plus:
        return "+";
    case InfixExpression::Operator::Minus:
        return "-";
    case InfixExpression::Operator::Asterisk:
        return "*";
    case InfixExpression::Operator::Slash:
        return "/";
    case InfixExpression::Operator::Lt:
        return "<";
    case InfixExpression::Operator::Gt:
        return ">";
    case InfixExpression::Operator::Eq:
        return "==";
    case InfixExpression::Operator::NotEq:
        return "!=";
    }

    unreachable;
    return "";
}
