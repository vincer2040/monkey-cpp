#include "../src/lexer.hh"
#include "../src/token.hh"
#include <gtest/gtest.h>

struct TokenTest {
    Token::Type type;
    const char* literal;
};

TEST(AddTest, SimpleAssertions) {
    std::string input = "let five = 5;\n\
let ten = 10;\n\
let add = fn(x, y) {\n\
x + y;\n\
};\n\
let result = add(five, ten);\n\
!-/*5;\n\
5 < 10 > 5;\n\
if (5 < 10) {\n\
    return true;\n\
} else {\n\
    return false;\n\
}\n\
10 == 10;\n\
10 != 9;\n\
";
    TokenTest tests[] = {
        {Token::Type::Let, "let"},     {Token::Type::Ident, "five"},
        {Token::Type::Assign, "="},    {Token::Type::Int, "5"},
        {Token::Type::Semicolon, ";"}, {Token::Type::Let, "let"},
        {Token::Type::Ident, "ten"},   {Token::Type::Assign, "="},
        {Token::Type::Int, "10"},      {Token::Type::Semicolon, ";"},
        {Token::Type::Let, "let"},     {Token::Type::Ident, "add"},
        {Token::Type::Assign, "="},    {Token::Type::Function, "fn"},
        {Token::Type::LParen, "("},    {Token::Type::Ident, "x"},
        {Token::Type::Comma, ","},     {Token::Type::Ident, "y"},
        {Token::Type::RParen, ")"},    {Token::Type::LSquirly, "{"},
        {Token::Type::Ident, "x"},     {Token::Type::Plus, "+"},
        {Token::Type::Ident, "y"},     {Token::Type::Semicolon, ";"},
        {Token::Type::RSquirly, "}"},  {Token::Type::Semicolon, ";"},
        {Token::Type::Let, "let"},     {Token::Type::Ident, "result"},
        {Token::Type::Assign, "="},    {Token::Type::Ident, "add"},
        {Token::Type::LParen, "("},    {Token::Type::Ident, "five"},
        {Token::Type::Comma, ","},     {Token::Type::Ident, "ten"},
        {Token::Type::RParen, ")"},    {Token::Type::Semicolon, ";"},
        {Token::Type::Bang, "!"},      {Token::Type::Minus, "-"},
        {Token::Type::Slash, "/"},     {Token::Type::Asterisk, "*"},
        {Token::Type::Int, "5"},       {Token::Type::Semicolon, ";"},
        {Token::Type::Int, "5"},       {Token::Type::Lt, "<"},
        {Token::Type::Int, "10"},      {Token::Type::Gt, ">"},
        {Token::Type::Int, "5"},       {Token::Type::Semicolon, ";"},
        {Token::Type::If, "if"},       {Token::Type::LParen, "("},
        {Token::Type::Int, "5"},       {Token::Type::Lt, "<"},
        {Token::Type::Int, "10"},      {Token::Type::RParen, ")"},
        {Token::Type::LSquirly, "{"},  {Token::Type::Return, "return"},
        {Token::Type::True, "true"},   {Token::Type::Semicolon, ";"},
        {Token::Type::RSquirly, "}"},  {Token::Type::Else, "else"},
        {Token::Type::LSquirly, "{"},  {Token::Type::Return, "return"},
        {Token::Type::False, "false"}, {Token::Type::Semicolon, ";"},
        {Token::Type::RSquirly, "}"},  {Token::Type::Int, "10"},
        {Token::Type::Eq, "=="},       {Token::Type::Int, "10"},
        {Token::Type::Semicolon, ";"}, {Token::Type::Int, "10"},
        {Token::Type::NotEq, "!="},    {Token::Type::Int, "9"},
        {Token::Type::Semicolon, ";"}, {Token::Type::Eof, ""},
    };
    size_t i, len = sizeof tests / sizeof tests[0];
    Lexer l(input);
    for (i = 0; i < len; ++i) {
        Token tok = l.next_token();
        TokenTest test = tests[i];
        EXPECT_EQ(tok.type, test.type);
        EXPECT_STREQ(tok.get_literal(), test.literal);
    }
}
