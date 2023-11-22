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

const char* Token::token_type_string() {
    switch (type) {
    case Type::Illegal:
        return "Illegal";
    case Type::Eof:
        return "Eof";
    case Type::Ident:
        return "Ident";
    case Type::Int:
        return "Int";
    case Type::Assign:
        return "Assign";
    case Type::Plus:
        return "Plus";
    case Type::Bang:
        return "Bang";
    case Type::Minus:
        return "Minus";
    case Type::Slash:
        return "Slash";
    case Type::Asterisk:
        return "Asterisk";
    case Type::Lt:
        return "Lt";
    case Type::Gt:
        return "Gt";
    case Type::Eq:
        return "Eq";
    case Type::NotEq:
        return "NotEq";
    case Type::LParen:
        return "LParen";
    case Type::RParen:
        return "RParen";
    case Type::LSquirly:
        return "LSquirly";
    case Type::RSquirly:
        return "RSquirly";
    case Type::Comma:
        return "Comma";
    case Type::Semicolon:
        return "Semicolon";
    case Type::Function:
        return "Function";
    case Type::Let:
        return "Let";
    case Type::If:
        return "If";
    case Type::Else:
        return "Else";
    case Type::Return:
        return "Return";
    case Type::True:
        return "True";
    case Type::False:
        return "False";
    }
    unreachable;
}

const char* token_type_to_string(Token::Type type) {
    switch (type) {
    case Token::Type::Illegal:
        return "Illegal";
    case Token::Type::Eof:
        return "Eof";
    case Token::Type::Ident:
        return "Ident";
    case Token::Type::Int:
        return "Int";
    case Token::Type::Assign:
        return "Assign";
    case Token::Type::Plus:
        return "Plus";
    case Token::Type::Bang:
        return "Bang";
    case Token::Type::Minus:
        return "Minus";
    case Token::Type::Slash:
        return "Slash";
    case Token::Type::Asterisk:
        return "Asterisk";
    case Token::Type::Lt:
        return "Lt";
    case Token::Type::Gt:
        return "Gt";
    case Token::Type::Eq:
        return "Eq";
    case Token::Type::NotEq:
        return "NotEq";
    case Token::Type::LParen:
        return "LParen";
    case Token::Type::RParen:
        return "RParen";
    case Token::Type::LSquirly:
        return "LSquirly";
    case Token::Type::RSquirly:
        return "RSquirly";
    case Token::Type::Comma:
        return "Comma";
    case Token::Type::Semicolon:
        return "Semicolon";
    case Token::Type::Function:
        return "Function";
    case Token::Type::Let:
        return "Let";
    case Token::Type::If:
        return "If";
    case Token::Type::Else:
        return "Else";
    case Token::Type::Return:
        return "Return";
    case Token::Type::True:
        return "True";
    case Token::Type::False:
        return "False";
    }
    unreachable;
}
