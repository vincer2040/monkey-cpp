#include "token.hh"
#include "util.hh"

struct KeyWordMapItem {
    const char* literal;
    Token::Type type;
};

const KeyWordMapItem key_words[] = {
    {"fn", Token::Type::Function},   {"if", Token::Type::If},
    {"let", Token::Type::Let},       {"else", Token::Type::Else},
    {"true", Token::Type::True},     {"false", Token::Type::False},
    {"return", Token::Type::Return},
};

size_t key_words_len = sizeof key_words / sizeof key_words[0];

Token::Type lookup_ident(const std::string& ident) {
    size_t i;
    for (i = 0; i < key_words_len; ++i) {
        KeyWordMapItem item = key_words[i];
        if (item.literal == ident) {
            return item.type;
        }
    }
    return Token::Type::Ident;
}

const char* Token::get_literal() {
    switch (type) {
    case Type::Illegal:
        return "Illegal";
    case Type::Eof:
        return "";
    case Type::Assign:
        return "=";
    case Type::Plus:
        return "+";
    case Type::LParen:
        return "(";
    case Type::RParen:
        return ")";
    case Type::LSquirly:
        return "{";
    case Type::RSquirly:
        return "}";
    case Type::Function:
        return "fn";
    case Type::Let:
        return "let";
    case Type::Comma:
        return ",";
    case Type::Semicolon:
        return ";";
    case Type::Bang:
        return "!";
    case Type::Minus:
        return "-";
    case Type::Slash:
        return "/";
    case Type::Asterisk:
        return "*";
    case Type::Lt:
        return "<";
    case Type::Gt:
        return ">";
    case Type::Eq:
        return "==";
    case Type::NotEq:
        return "!=";
    case Type::If:
        return "if";
    case Type::Else:
        return "else";
    case Type::Return:
        return "return";
    case Type::True:
        return "true";
    case Type::False:
        return "false";
    case Type::Ident: {
        auto lit = std::get<std::shared_ptr<std::string>>(literal);
        return lit->c_str();
    }
    case Type::Int: {
        auto lit = std::get<std::shared_ptr<std::string>>(literal);
        return lit->c_str();
    }
    }

    unreachable;
}
