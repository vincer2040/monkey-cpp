#include "lexer.hh"
#include "token.hh"
#include "util.hh"
#include <memory>

Lexer::Lexer(const std::string& input) : input(input), pos(0), ch(0) {
    read_char();
}

Token Lexer::next_token() {
    Token tok;
    skip_whitespace();
    switch (ch) {
    case '=':
        if (peek_char() == '=') {
            read_char();
            tok.type = Token::Type::Eq;
        } else {
            tok.type = Token::Type::Assign;
        }
        break;
    case '+':
        tok.type = Token::Type::Plus;
        break;
    case ';':
        tok.type = Token::Type::Semicolon;
        break;
    case ',':
        tok.type = Token::Type::Comma;
        break;
    case '(':
        tok.type = Token::Type::LParen;
        break;
    case ')':
        tok.type = Token::Type::RParen;
        break;
    case '{':
        tok.type = Token::Type::LSquirly;
        break;
    case '}':
        tok.type = Token::Type::RSquirly;
        break;
    case '!':
        if (peek_char() == '=') {
            tok.type = Token::Type::NotEq;
            read_char();
        } else {
            tok.type = Token::Type::Bang;
        }
        break;
    case '-':
        tok.type = Token::Type::Minus;
        break;
    case '/':
        tok.type = Token::Type::Slash;
        break;
    case '*':
        tok.type = Token::Type::Asterisk;
        break;
    case '<':
        tok.type = Token::Type::Lt;
        break;
    case '>':
        tok.type = Token::Type::Gt;
        break;
    case 0:
        tok.type = Token::Type::Eof;
        break;
    default:
        if (is_letter(ch)) {
            std::string literal = read_ident();
            Token::Type type = lookup_ident(literal);
            if (type == Token::Type::Ident) {
                tok.literal = std::make_shared<std::string>(literal);
            }
            tok.type = type;
            return tok;
        } else if (is_digit(ch)) {
            tok.type = Token::Type::Int;
            tok.literal = std::make_shared<std::string>(read_int());
            return tok;
        }
        tok.type = Token::Type::Illegal;
        break;
    }
    read_char();
    return tok;
}

char Lexer::peek_char() {
    if (pos >= input.size()) {
        return 0;
    }
    return input[pos];
}

std::string Lexer::read_ident() {
    std::string res;
    while (is_letter(ch)) {
        res.push_back(ch);
        read_char();
    }
    return res;
}

std::string Lexer::read_int() {
    std::string res;
    while (is_digit(ch)) {
        res.push_back(ch);
        read_char();
    }
    return res;
}

void Lexer::skip_whitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        read_char();
    }
}

void Lexer::read_char() {
    if (pos >= input.size()) {
        ch = 0;
    } else {
        ch = input[pos];
    }
    pos++;
}
