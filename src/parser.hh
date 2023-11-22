#pragma once

#include "ast.hh"
#include "lexer.hh"
#include "token.hh"

enum class Precedence {
    Lowest = 0,
    Equals = 1,      // ==
    LessGreater = 2, // > or <
    Sum = 3,         // +
    Product = 4,     // *
    Prefix = 5,      // -X or !X
    Call = 6,        // myFunction(X)
};

class Parser {
  public:
    Parser(Lexer& l);
    Program parse();
    std::vector<std::string>& get_errors();

  private:
    Lexer& l;
    Token cur;
    Token peek;
    std::vector<std::string> errors;
    Statement parse_statement();
    Statement parse_let_statement();
    Statement parse_return_statement();
    Statement parse_expression_statement();
    Expression parse_expression(Precedence precedence);
    Expression parse_identifier();
    Expression parse_integer();
    Expression parse_boolean();
    Expression parse_prefix();
    Expression parse_infix(Expression& left);
    Expression parse_group();
    Expression parse_if();
    Expression parse_function();
    Expression parse_call(Expression& function);
    BlockStatement parse_block();
    std::vector<Identifier> parse_function_params();
    std::vector<Expression> parse_call_args();
    void next_token();
    bool cur_tok_is(Token::Type type);
    bool peek_tok_is(Token::Type type);
    bool expect_peek(Token::Type type);
    Precedence precedence(Token::Type type);
    Precedence cur_precedence();
    Precedence peek_precedence();
    void peek_error(Token::Type type);
    void no_prefix_parse_method(Token::Type type);
};
