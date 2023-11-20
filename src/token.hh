#pragma once

#include <memory>
#include <string>
#include <variant>

struct Token {
    enum class Type {
        Illegal,
        Eof,
        Ident,
        Int,
        Assign,
        Plus,
        Bang,
        Minus,
        Slash,
        Asterisk,
        Lt,
        Gt,
        Eq,
        NotEq,
        LParen,
        RParen,
        LSquirly,
        RSquirly,
        Comma,
        Semicolon,
        Function,
        Let,
        If,
        Else,
        Return,
        True,
        False,
    } type;
    std::variant<std::monostate, std::shared_ptr<std::string>> literal;
    const char* get_literal();
};

Token::Type lookup_ident(const std::string& ident);
