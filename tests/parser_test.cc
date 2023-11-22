#include "../src/ast.hh"
#include "../src/lexer.hh"
#include "../src/parser.hh"
#include <gtest/gtest.h>
#include <iostream>

#define arr_size(arr) sizeof arr / sizeof arr[0]

#define check_errors(p)                                                        \
    do {                                                                       \
        std::vector<std::string> errs = p.get_errors();                        \
        if (errs.size() > 0) {                                                 \
            for (auto& e : errs) {                                             \
                std::cout << e << "\n";                                        \
            }                                                                  \
            EXPECT_EQ(errs.size(), 0);                                         \
        }                                                                      \
    } while (0)

#define test_let_statement(stmt, name)                                         \
    do {                                                                       \
        EXPECT_EQ(stmt.type, Statement::Type::Let);                            \
        auto let_stmt = std::get<LetStatement>(stmt.data);                     \
        EXPECT_STREQ(let_stmt.token_literal(), "let");                         \
        EXPECT_STREQ(let_stmt.name.token_literal(), name);                     \
        EXPECT_STREQ(let_stmt.name.value->c_str(), name);                      \
    } while (0)

#define test_ident(ident, name)                                                \
    do {                                                                       \
        EXPECT_STREQ(ident.value->c_str(), name);                              \
        EXPECT_STREQ(ident.token_literal(), name);                             \
    } while (0)

#define test_identifier(e, name)                                               \
    do {                                                                       \
        EXPECT_EQ(e.type, Expression::Type::Identifier);                       \
        auto ident = std::get<Identifier>(e.data);                             \
        EXPECT_STREQ(ident.value->c_str(), name);                              \
        EXPECT_STREQ(ident.token_literal(), name);                             \
    } while (0)

#define test_integer_literal(e, val, lit)                                      \
    do {                                                                       \
        EXPECT_EQ(e.type, Expression::Type::Integer);                          \
        auto il = std::get<IntegerLiteral>(e.data);                            \
        EXPECT_EQ(il.value, val);                                              \
        EXPECT_STREQ(il.token_literal(), lit);                                 \
    } while (0);

#define test_boolean_literal(e, val, lit)                                      \
    do {                                                                       \
        EXPECT_EQ(e.type, Expression::Type::Boolean);                          \
        auto il = std::get<BooleanLiteral>(e.data);                            \
        EXPECT_EQ(il.value, val);                                              \
        EXPECT_STREQ(il.token_literal(), lit);                                 \
    } while (0);

#define test_literal(type, e, val, lit)                                        \
    do {                                                                       \
        switch (type) {                                                        \
        case TestType::Int:                                                    \
            test_integer_literal(e, std::get<int64_t>(val), lit);              \
            break;                                                             \
        case TestType::Bool:                                                   \
            test_boolean_literal(e, std::get<bool>(val), lit);                 \
            break;                                                             \
        }                                                                      \
    } while (0);

enum class TestType {
    Int,
    Bool,
};

struct LetTest {
    TestType type;
    const char* exp_name;
    std::variant<int64_t, bool> value;
    const char* exp_lit;
};

struct PrefixTest {
    TestType type;
    std::string input;
    PrefixExpression::Operator oper;
    std::variant<int64_t, bool> value;
    const char* lit;
};

struct PrecedenceTest {
    std::string input;
    const char* exp;
};

struct InfixTest {
    TestType type;
    std::string input;
    InfixExpression::Operator oper;
    std::variant<int64_t, bool> lval;
    std::variant<int64_t, bool> rval;
    const char* lval_lit;
    const char* rval_lit;
};

struct FunctionParamsTest {
    std::string input;
    size_t exp_len;
    const char* exps[3];
};

TEST(Parser, LetStatement) {
    std::string input = "\
let x = 5;\n\
let y = 10;\n\
let foobar = true;\n\
";

    LetTest tests[] = {
        {TestType::Int, "x", 5, "5"},
        {TestType::Int, "y", 10, "10"},
        {TestType::Bool, "foobar", true, "true"},
    };
    size_t i, len = arr_size(tests);
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 3);

    for (i = 0; i < len; ++i) {
        Statement stmt = program.statements[i];
        const char* name = tests[i].exp_name;
        test_let_statement(stmt, name);
        auto let = std::get<LetStatement>(stmt.data);
        test_literal(tests[i].type, let.value, tests[i].value,
                     tests[i].exp_lit);
    }
}

TEST(Parser, ReturnStatement) {
    std::string input = "\
return 5;\n\
return 10;\n\
return 993322;\n\
";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 3);

    for (auto& stmt : program.statements) {
        EXPECT_EQ(stmt.type, Statement::Type::Ret);
        EXPECT_STREQ(stmt.token_literal(), "return");
    }
}

TEST(Parser, Identifier) {
    std::string input = "foobar;";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    Statement stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto e = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(e.type, Expression::Type::Identifier);
    auto i = std::get<Identifier>(e.data);
    EXPECT_STREQ(i.value->c_str(), "foobar");
    EXPECT_STREQ(i.token_literal(), "foobar");
}

TEST(Parser, Integer) {
    std::string input = "5;";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    Statement stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto e = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(e.type, Expression::Type::Integer);
    auto i = std::get<IntegerLiteral>(e.data);
    EXPECT_EQ(i.value, 5);
    EXPECT_STREQ(i.token_literal(), "5");
}

TEST(Parser, Boolean) {
    std::string input = "true";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    Statement stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto e = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(e.type, Expression::Type::Boolean);
    auto i = std::get<BooleanLiteral>(e.data);
    EXPECT_EQ(i.value, true);
    EXPECT_STREQ(i.token_literal(), "true");
}

TEST(Parser, PrefixExpression) {
    PrefixTest tests[] = {
        {TestType::Int, "!5;", PrefixExpression::Operator::Bang, 5, "5"},
        {TestType::Int, "-15;", PrefixExpression::Operator::Minus, 15, "15"},
        {TestType::Bool, "!true;", PrefixExpression::Operator::Bang, true,
         "true"},
        {TestType::Bool, "!false;", PrefixExpression::Operator::Bang, false,
         "false"},
    };

    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        PrefixTest test = tests[i];
        Lexer l(test.input);
        Parser p(l);
        Program program = p.parse();
        check_errors(p);
        EXPECT_EQ(program.statements.size(), 1);
        Statement stmt = program.statements[0];
        EXPECT_EQ(stmt.type, Statement::Type::Expression);
        auto e = std::get<ExpressionStatement>(stmt.data).exp;
        EXPECT_EQ(e.type, Expression::Type::Prefix);
        auto pe = std::get<PrefixExpression>(e.data);
        EXPECT_EQ(pe.oper, test.oper);
        test_literal(test.type, (*(pe.right)), test.value, test.lit);
    }
}

TEST(Parser, InfixExpression) {
    InfixTest tests[] = {
        {TestType::Int, "5 + 5;", InfixExpression::Operator::Plus, 5, 5, "5",
         "5"},
        {TestType::Int, "5 - 5;", InfixExpression::Operator::Minus, 5, 5, "5",
         "5"},
        {TestType::Int, "5 * 5;", InfixExpression::Operator::Asterisk, 5, 5,
         "5", "5"},
        {TestType::Int, "5 / 5;", InfixExpression::Operator::Slash, 5, 5, "5",
         "5"},
        {TestType::Int, "5 > 5;", InfixExpression::Operator::Gt, 5, 5, "5",
         "5"},
        {TestType::Int, "5 < 5;", InfixExpression::Operator::Lt, 5, 5, "5",
         "5"},
        {TestType::Int, "5 == 5;", InfixExpression::Operator::Eq, 5, 5, "5",
         "5"},
        {TestType::Int, "5 != 5;", InfixExpression::Operator::NotEq, 5, 5, "5",
         "5"},
        {TestType::Bool, "true == true", InfixExpression::Operator::Eq, true,
         true, "true", "true"},
        {TestType::Bool, "true != false", InfixExpression::Operator::NotEq,
         true, false, "true", "false"},
        {TestType::Bool, "false == false", InfixExpression::Operator::Eq, false,
         false, "false", "false"},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        InfixTest test = tests[i];
        Lexer l(test.input);
        Parser p(l);
        Program program = p.parse();
        check_errors(p);
        auto stmt = program.statements[0];
        EXPECT_EQ(stmt.type, Statement::Type::Expression);
        auto e = std::get<ExpressionStatement>(stmt.data).exp;
        EXPECT_EQ(e.type, Expression::Type::Infix);
        auto ie = std::get<InfixExpression>(e.data);
        EXPECT_EQ(ie.oper, test.oper);
        test_literal(test.type, (*ie.left), test.lval, test.lval_lit);
        test_literal(test.type, (*ie.right), test.rval, test.rval_lit);
    }
}

TEST(Parser, OperatorPrecedence) {
    PrecedenceTest tests[] = {
        {"-a * b", "((-a) * b)"},
        {"!-a", "(!(-a))"},
        {"a + b + c", "((a + b) + c)"},
        {"a + b - c", "((a + b) - c)"},
        {"a * b * c", "((a * b) * c)"},
        {"a * b / c", "((a * b) / c)"},
        {"a + b / c", "(a + (b / c))"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
        {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
        {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5",
         "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5",
         "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"true", "true"},
        {"false", "false"},
        {"3 > 5 == false", "((3 > 5) == false)"},
        {"3 < 5 == true", "((3 < 5) == true)"},
        {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
        {"(5 + 5) * 2", "((5 + 5) * 2)"},
        {"2 / (5 + 5)", "(2 / (5 + 5))"},
        {"-(5 + 5)", "(-(5 + 5))"},
        {"!(true == true)", "(!(true == true))"},
        {"a + add(b * c) + d", "((a + add((b * c))) + d)"},
        {"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
         "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
        {"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"},
    };

    size_t i, len = arr_size(tests);

    for (i = 0; i < len; ++i) {
        PrecedenceTest test = tests[i];
        Lexer l(test.input);
        Parser p(l);
        Program program = p.parse();
        check_errors(p);
        std::string program_string = program.string();
        EXPECT_STREQ(program_string.c_str(), test.exp);
    }
}

TEST(Parser, IfExpression) {
    std::string input = "if (x < y) { x }";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    auto stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto e = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(e.type, Expression::Type::If);
    auto ife = std::get<IfExpression>(e.data);
    EXPECT_EQ(ife.condition->type, Expression::Type::Infix);
    auto infix = std::get<InfixExpression>(ife.condition->data);
    test_identifier((*infix.left), "x");
    test_identifier((*infix.right), "y");

    EXPECT_EQ(ife.consequence.stmts.size(), 1);
    auto conseq_stmt = ife.consequence.stmts[0];
    EXPECT_EQ(conseq_stmt.type, Statement::Type::Expression);
    auto conseq_exp = std::get<ExpressionStatement>(conseq_stmt.data).exp;
    test_identifier(conseq_exp, "x");
    EXPECT_EQ(ife.alternative.has_value(), false);
}

TEST(Parser, IfElseExpression) {
    std::string input = "if (x < y) { x } else { y }";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    auto stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto e = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(e.type, Expression::Type::If);
    auto ife = std::get<IfExpression>(e.data);
    EXPECT_EQ(ife.condition->type, Expression::Type::Infix);
    auto infix = std::get<InfixExpression>(ife.condition->data);
    test_identifier((*infix.left), "x");
    test_identifier((*infix.right), "y");

    EXPECT_EQ(ife.consequence.stmts.size(), 1);
    auto conseq_stmt = ife.consequence.stmts[0];
    EXPECT_EQ(conseq_stmt.type, Statement::Type::Expression);
    auto conseq_exp = std::get<ExpressionStatement>(conseq_stmt.data).exp;
    test_identifier(conseq_exp, "x");

    EXPECT_EQ(ife.alternative.has_value(), true);
    EXPECT_EQ(ife.alternative->stmts.size(), 1);
    auto alt_stmt = ife.alternative->stmts[0];
    EXPECT_EQ(alt_stmt.type, Statement::Type::Expression);
    auto alt_exp = std::get<ExpressionStatement>(alt_stmt.data).exp;
    test_identifier(alt_exp, "y");
}

TEST(Parser, Functions) {
    std::string input = "fn(x, y) { x + y }";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    auto stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto e = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(e.type, Expression::Type::Function);
    auto fn = std::get<FunctionLiteral>(e.data);
    EXPECT_EQ(fn.params.size(), 2);
    test_ident(fn.params[0], "x");
    test_ident(fn.params[1], "y");
    EXPECT_EQ(fn.body.string(), "(x + y)");
}

TEST(Parser, FunctionParams) {
    FunctionParamsTest tests[] = {
        {"fn() {}", 0, {}},
        {"fn(x) {}", 1, {"x"}},
        {"fn(x, y, z) {}", 3, {"x", "y", "z"}},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        FunctionParamsTest test = tests[i];
        Lexer l(test.input);
        Parser p(l);
        Program program = p.parse();
        size_t j;
        check_errors(p);
        EXPECT_EQ(program.statements.size(), 1);
        auto stmt = program.statements[0];
        EXPECT_EQ(stmt.type, Statement::Type::Expression);
        auto e = std::get<ExpressionStatement>(stmt.data).exp;
        EXPECT_EQ(e.type, Expression::Type::Function);
        auto fn = std::get<FunctionLiteral>(e.data);
        EXPECT_EQ(fn.params.size(), test.exp_len);
        for (j = 0; j < test.exp_len; ++j) {
            const char* exp = test.exps[j];
            auto param = fn.params[j];
            test_ident(param, exp);
        }
    }
}

TEST(Parser, Call) {
    std::string input = "add(1, 2 * 3, 4 + 5)";
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    check_errors(p);
    EXPECT_EQ(program.statements.size(), 1);
    auto stmt = program.statements[0];
    EXPECT_EQ(stmt.type, Statement::Type::Expression);
    auto exp = std::get<ExpressionStatement>(stmt.data).exp;
    EXPECT_EQ(exp.type, Expression::Type::Call);
    auto call = std::get<CallExpression>(exp.data);
    test_identifier((*call.function), "add");
    EXPECT_EQ(call.arguments.size(), 3);
}
