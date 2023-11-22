#pragma once

#include "token.hh"
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

class Node {
  public:
    virtual const char* token_literal() = 0;
    virtual std::string string() = 0;
};

struct Identifier : Node {
    Token tok; /* the Ident token */
    std::shared_ptr<std::string> value;
    Identifier(Token tok, std::shared_ptr<std::string> value);
    const char* token_literal() override;
    std::string string() override;
};

struct IntegerLiteral : Node {
    Token tok; /* the Int token */
    int64_t value;
    IntegerLiteral(Token tok, int64_t value);
    const char* token_literal() override;
    std::string string() override;
};

struct BooleanLiteral : Node {
    Token tok;
    bool value;
    BooleanLiteral(Token tok, bool value);
    const char* token_literal() override;
    std::string string() override;
};

struct PrefixExpression : Node {
    Token tok; /* ! or - */
    enum class Operator {
        Bang,
        Minus,
    } oper;
    std::shared_ptr<struct Expression> right;
    PrefixExpression(Token tok, PrefixExpression::Operator oper,
                     struct Expression& right);
    const char* token_literal() override;
    std::string string() override;
};

struct InfixExpression : Node {
    Token tok; /* operator token */
    enum class Operator {
        Plus,
        Minus,
        Asterisk,
        Slash,
        Lt,
        Gt,
        Eq,
        NotEq,
    } oper;
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    InfixExpression(Token tok, Operator oper, Expression& left,
                    Expression& right);
    const char* token_literal() override;
    std::string string() override;
};

struct BlockStatement : Node {
    Token tok; /* the { token */
    std::vector<struct Statement> stmts;
    BlockStatement();
    BlockStatement(Token tok, std::vector<struct Statement>& stmts);
    const char* token_literal() override;
    std::string string() override;
};

struct IfExpression : Node {
    Token tok; /* if token */
    std::shared_ptr<Expression> condition;
    BlockStatement consequence;
    std::optional<BlockStatement> alternative;
    IfExpression(Token tok, Expression& condition, BlockStatement& consequence,
                 std::optional<BlockStatement>& alternative);
    const char* token_literal() override;
    std::string string() override;
};

struct FunctionLiteral : Node {
    Token tok; /* the fn token */
    std::vector<Identifier> params;
    BlockStatement body;
    FunctionLiteral(Token tok, std::vector<Identifier>& params,
                    BlockStatement& body);
    const char* token_literal() override;
    std::string string() override;
};

struct CallExpression : Node {
    Token tok;
    std::shared_ptr<struct Expression> function;
    std::vector<struct Expression> arguments;
    CallExpression(Token tok, struct Expression& function,
                   std::vector<struct Expression>& arguments);
    const char* token_literal() override;
    std::string string() override;
};

typedef std::variant<std::monostate, Identifier, BooleanLiteral, IntegerLiteral,
                     PrefixExpression, InfixExpression, IfExpression,
                     FunctionLiteral, CallExpression>
    ExpressionVariant;

struct Expression : Node {
    enum class Type {
        Inv,
        Identifier,
        Integer,
        Boolean,
        Prefix,
        Infix,
        If,
        Function,
        Call,
    } type;
    ExpressionVariant data;
    Expression();
    Expression(Expression::Type type, ExpressionVariant data);
    const char* token_literal() override;
    std::string string() override;
};

struct LetStatement : Node {
    Token tok; /* the Let token */
    Identifier name;
    Expression value;
    LetStatement(Token tok, Identifier name, Expression& value);
    const char* token_literal() override;
    std::string string() override;
};

struct ReturnStatement : Node {
    Token tok; /* the Return token */
    Expression value;
    ReturnStatement(Token tok, Expression& value);
    const char* token_literal() override;
    std::string string() override;
};

struct ExpressionStatement : Node {
    Token tok; /* first token of expression */
    Expression exp;
    ExpressionStatement(Token tok, Expression e);
    const char* token_literal() override;
    std::string string() override;
};

struct Statement : Node {
    enum class Type {
        Inv,
        Let,
        Ret,
        Expression,
    } type;
    std::variant<std::monostate, LetStatement, ReturnStatement,
                 ExpressionStatement>
        data;
    Statement();
    const char* token_literal() override;
    std::string string() override;
};

struct Program : Node {
    std::vector<Statement> statements;
    const char* token_literal() override;
    std::string string() override;
};

const char* prefix_oper_to_string(PrefixExpression::Operator oper);
const char* infix_oper_to_string(InfixExpression::Operator oper);
