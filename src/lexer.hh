#pragma once

#include "token.hh"
#include <string>

class Lexer {
  public:
    Lexer(const std::string& input);
    Token next_token();

  private:
    const std::string& input;
    size_t pos;
    char ch;
    void read_char();
    void skip_whitespace();
    char peek_char();
    std::string read_ident();
    std::string read_int();
};
