#include "parser.hh"
#include "ast.hh"
#include "util.hh"
#include <vector>

Parser::Parser(Lexer& l) : l(l) {
    next_token();
    next_token();
}

Program Parser::parse() {
    Program program;
    while (cur.type != Token::Type::Eof) {
        Statement stmt = parse_statement();
        if (stmt.type != Statement::Type::Inv) {
            program.statements.push_back(stmt);
        }
        next_token();
    }
    return program;
}

std::vector<std::string>& Parser::get_errors() { return errors; }

Statement Parser::parse_statement() {
    Statement stmt;
    switch (cur.type) {
    case Token::Type::Let:
        stmt = parse_let_statement();
        break;
    case Token::Type::Return:
        stmt = parse_return_statement();
        break;
    default:
        stmt = parse_expression_statement();
        break;
    }
    return stmt;
}

Statement Parser::parse_let_statement() {
    Statement stmt;
    Token let_tok = cur;
    if (!expect_peek(Token::Type::Ident)) {
        return stmt;
    }
    Identifier name(cur, std::get<std::shared_ptr<std::string>>(cur.literal));
    if (!expect_peek(Token::Type::Assign)) {
        return stmt;
    }
    next_token();
    Expression value = parse_expression(Precedence::Lowest);
    stmt.type = Statement::Type::Let;
    stmt.data = LetStatement(let_tok, name, value);
    if (peek_tok_is(Token::Type::Semicolon)) {
        next_token();
    }
    return stmt;
}

Statement Parser::parse_return_statement() {
    Statement stmt;
    Token ret_tok = cur;
    next_token();
    Expression value = parse_expression(Precedence::Lowest);
    ReturnStatement ret_stmt(ret_tok, value);
    if (peek_tok_is(Token::Type::Semicolon)) {
        next_token();
    }
    stmt.type = Statement::Type::Ret;
    stmt.data = ret_stmt;
    return stmt;
}

Statement Parser::parse_expression_statement() {
    Statement stmt;
    stmt.type = Statement::Type::Expression;
    stmt.data = ExpressionStatement(cur, parse_expression(Precedence::Lowest));
    if (peek_tok_is(Token::Type::Semicolon)) {
        next_token();
    }
    return stmt;
}

Expression Parser::parse_expression(Precedence precedence) {
    Expression e;
    switch (cur.type) {
    case Token::Type::Ident:
        e = parse_identifier();
        break;
    case Token::Type::Int:
        e = parse_integer();
        break;
    case Token::Type::True:
        e = parse_boolean();
        break;
    case Token::Type::False:
        e = parse_boolean();
        break;
    case Token::Type::Bang:
        e = parse_prefix();
        break;
    case Token::Type::Minus:
        e = parse_prefix();
        break;
    case Token::Type::LParen:
        e = parse_group();
        break;
    case Token::Type::If:
        e = parse_if();
        break;
    case Token::Type::Function:
        e = parse_function();
        break;
    default:
        no_prefix_parse_method(cur.type);
        return e;
    }

    while (!peek_tok_is(Token::Type::Semicolon) &&
           precedence < peek_precedence()) {
        switch (peek.type) {
        case Token::Type::Plus:
        case Token::Type::Minus:
        case Token::Type::Asterisk:
        case Token::Type::Slash:
        case Token::Type::Lt:
        case Token::Type::Gt:
        case Token::Type::Eq:
        case Token::Type::NotEq:
            next_token();
            e = parse_infix(e);
            break;
        case Token::Type::LParen:
            next_token();
            e = parse_call(e);
            break;
        default:
            return e;
        }
    }
    return e;
}

Expression Parser::parse_identifier() {
    Identifier ident(cur, std::get<std::shared_ptr<std::string>>(cur.literal));
    return Expression(Expression::Type::Identifier, ident);
}

Expression Parser::parse_integer() {
    int64_t value = 0;
    auto str = *std::get<std::shared_ptr<std::string>>(cur.literal);
    for (auto c : str) {
        value = (value * 10) + (c - '0');
    }
    IntegerLiteral i(cur, value);
    return Expression(Expression::Type::Integer, i);
}

Expression Parser::parse_boolean() {
    Token tok = cur;
    bool value = cur_tok_is(Token::Type::True);
    BooleanLiteral b(tok, value);
    return Expression(Expression::Type::Boolean, b);
}

Expression Parser::parse_prefix() {
    Token tok = cur;
    PrefixExpression::Operator oper;
    switch (cur.type) {
    case Token::Type::Bang:
        oper = PrefixExpression::Operator::Bang;
        break;
    case Token::Type::Minus:
        oper = PrefixExpression::Operator::Minus;
        break;
    default:
        unreachable;
    }
    next_token();
    Expression right = parse_expression(Precedence::Prefix);
    return Expression(Expression::Type::Prefix,
                      PrefixExpression(tok, oper, right));
}

Expression Parser::parse_infix(Expression& left) {
    Token tok = cur;
    InfixExpression::Operator oper;
    switch (cur.type) {
    case Token::Type::Plus:
        oper = InfixExpression::Operator::Plus;
        break;
    case Token::Type::Minus:
        oper = InfixExpression::Operator::Minus;
        break;
    case Token::Type::Asterisk:
        oper = InfixExpression::Operator::Asterisk;
        break;
    case Token::Type::Slash:
        oper = InfixExpression::Operator::Slash;
        break;
    case Token::Type::Lt:
        oper = InfixExpression::Operator::Lt;
        break;
    case Token::Type::Gt:
        oper = InfixExpression::Operator::Gt;
        break;
    case Token::Type::Eq:
        oper = InfixExpression::Operator::Eq;
        break;
    case Token::Type::NotEq:
        oper = InfixExpression::Operator::NotEq;
        break;
    default:
        unreachable;
    }

    Precedence precedence = cur_precedence();
    next_token();
    Expression right = parse_expression(precedence);
    InfixExpression ie(tok, oper, left, right);
    return Expression(Expression::Type::Infix, ie);
}

Expression Parser::parse_group() {
    next_token();
    Expression e = parse_expression(Precedence::Lowest);
    if (!expect_peek(Token::Type::RParen)) {
        return Expression();
    }
    return e;
}

Expression Parser::parse_if() {
    Token tok = cur;
    if (!expect_peek(Token::Type::LParen)) {
        return Expression();
    }
    next_token();
    Expression condition = parse_expression(Precedence::Lowest);
    if (!expect_peek(Token::Type::RParen)) {
        return Expression();
    }
    if (!expect_peek(Token::Type::LSquirly)) {
        return Expression();
    }
    BlockStatement consequence = parse_block();
    std::optional<BlockStatement> alternative = {};
    if (peek_tok_is(Token::Type::Else)) {
        next_token();
        if (!expect_peek(Token::Type::LSquirly)) {
            return Expression();
        }
        alternative = parse_block();
    }
    IfExpression ife(tok, condition, consequence, alternative);
    return Expression(Expression::Type::If, ife);
}

Expression Parser::parse_function() {
    Token tok = cur;
    if (!expect_peek(Token::Type::LParen)) {
        return Expression();
    }
    std::vector<Identifier> params = parse_function_params();
    if (!expect_peek(Token::Type::LSquirly)) {
        return Expression();
    }
    BlockStatement body = parse_block();
    FunctionLiteral fn(cur, params, body);
    return Expression(Expression::Type::Function, fn);
}

Expression Parser::parse_call(Expression& function) {
    Token tok = cur;
    std::vector<Expression> args = parse_call_args();
    CallExpression call(tok, function, args);
    return Expression(Expression::Type::Call, call);
}

BlockStatement Parser::parse_block() {
    BlockStatement block;
    block.tok = cur;
    next_token();

    while (!cur_tok_is(Token::Type::RSquirly) &&
           !cur_tok_is(Token::Type::Eof)) {
        Statement stmt = parse_statement();
        if (stmt.type != Statement::Type::Inv) {
            block.stmts.push_back(stmt);
        }
        next_token();
    }
    return block;
}

std::vector<Identifier> Parser::parse_function_params() {
    std::vector<Identifier> idents;
    if (peek_tok_is(Token::Type::RParen)) {
        next_token();
        return idents;
    }
    next_token();
    Identifier ident(cur, std::get<std::shared_ptr<std::string>>(cur.literal));
    idents.push_back(ident);

    while (peek_tok_is(Token::Type::Comma)) {
        next_token();
        next_token();
        ident.tok = cur;
        ident.value = std::get<std::shared_ptr<std::string>>(cur.literal);
        idents.push_back(ident);
    }

    if (!expect_peek(Token::Type::RParen)) {
        return std::vector<Identifier>();
    }
    return idents;
}

std::vector<Expression> Parser::parse_call_args() {
    std::vector<Expression> args;
    if (peek_tok_is(Token::Type::RParen)) {
        next_token();
        return args;
    }
    next_token();
    args.push_back(parse_expression(Precedence::Lowest));
    while (peek_tok_is(Token::Type::Comma)) {
        next_token();
        next_token();
        args.push_back(parse_expression(Precedence::Lowest));
    }
    if (!expect_peek(Token::Type::RParen)) {
        return std::vector<Expression>();
    }
    return args;
}

void Parser::next_token() {
    cur = peek;
    peek = l.next_token();
}

bool Parser::cur_tok_is(Token::Type type) { return cur.type == type; }

bool Parser::peek_tok_is(Token::Type type) { return peek.type == type; }

bool Parser::expect_peek(Token::Type type) {
    if (peek_tok_is(type)) {
        next_token();
        return true;
    }
    peek_error(type);
    return false;
}

Precedence Parser::precedence(Token::Type type) {
    switch (type) {
    case Token::Type::Eq:
        return Precedence::Equals;
    case Token::Type::NotEq:
        return Precedence::Equals;
    case Token::Type::Lt:
        return Precedence::LessGreater;
    case Token::Type::Gt:
        return Precedence::LessGreater;
    case Token::Type::Plus:
        return Precedence::Sum;
    case Token::Type::Minus:
        return Precedence::Sum;
    case Token::Type::Asterisk:
        return Precedence::Product;
    case Token::Type::Slash:
        return Precedence::Product;
    case Token::Type::LParen:
        return Precedence::Call;
    default:
        return Precedence::Lowest;
    }
    unreachable;
}

Precedence Parser::cur_precedence() { return precedence(cur.type); }

Precedence Parser::peek_precedence() { return precedence(peek.type); }

void Parser::peek_error(Token::Type type) {
    std::string err = "expected next token to be ";
    err.append(cur.token_type_string());
    err.append(", got ");
    err.append(token_type_to_string(type));
    err.append(" instead");
    errors.push_back(err);
}

void Parser::no_prefix_parse_method(Token::Type type) {
    std::string err = "no prefix parse function for ";
    err.append(token_type_to_string(type));
    err.append(" found");
    errors.push_back(err);
}
