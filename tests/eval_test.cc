#include "../src/ast.hh"
#include "../src/eval.hh"
#include "../src/lexer.hh"
#include "../src/object.hh"
#include "../src/parser.hh"
#include <gtest/gtest.h>

#define arr_size(arr) sizeof arr / sizeof arr[0]

#define test_int(obj, exp)                                                     \
    do {                                                                       \
        EXPECT_EQ(obj.type, Object::Type::Int);                                \
        EXPECT_EQ(std::get<int64_t>(obj.value), exp);                          \
    } while (0)

#define test_bool(obj, exp)                                                    \
    do {                                                                       \
        EXPECT_EQ(obj.type, Object::Type::Bool);                               \
        EXPECT_EQ(std::get<bool>(obj.value), exp);                             \
    } while (0)

#define test_null(obj)                                                         \
    do {                                                                       \
        EXPECT_EQ(obj.type, Object::Type::Null);                               \
    } while (0)

struct IntTest {
    std::string input;
    int64_t exp;
};

struct BoolTest {
    std::string input;
    bool exp;
};

struct IfElseTest {
    std::string input;
    bool has_exp;
    int64_t exp;
};

struct ErrorTest {
    std::string input;
    const char* exp;
};

static Object test_eval(const std::string& input) {
    Lexer l(input);
    Parser p(l);
    Program program = p.parse();
    std::shared_ptr<Environment> env =
        std::make_shared<Environment>(Environment());
    Object evaluated = eval(program, env);
    return evaluated;
}

TEST(Eval, Integers) {
    IntTest tests[] = {
        {"5", 5},
        {"10", 10},
        {"-5", -5},
        {"-10", -10},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"-50 + 100 + -50", 0},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"20 + 2 * -10", 0},
        {"50 / 2 * 2 + 10", 60},
        {"2 * (5 + 10)", 30},
        {"3 * 3 * 3 + 10", 37},
        {"3 * (3 * 3) + 10", 37},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        IntTest test = tests[i];
        Object evaluated = test_eval(test.input);
        test_int(evaluated, test.exp);
    }
}

TEST(Eval, Booleans) {
    BoolTest tests[] = {
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 1", true},
        {"1 != 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"true == true", true},
        {"false == false", true},
        {"true == false", false},
        {"true != false", true},
        {"false != true", true},
        {"(1 < 2) == true", true},
        {"(1 < 2) == false", false},
        {"(1 > 2) == true", false},
        {"(1 > 2) == false", true},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        BoolTest test = tests[i];
        Object evaluated = test_eval(test.input);
        test_bool(evaluated, test.exp);
    }
}

TEST(Eval, Bang) {
    BoolTest tests[] = {
        {"!true", false}, {"!false", true},   {"!5", false},
        {"!!true", true}, {"!!false", false}, {"!!5", true},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        BoolTest test = tests[i];
        Object evaluated = test_eval(test.input);
        test_bool(evaluated, test.exp);
    }
}

TEST(Eval, IfElse) {
    IfElseTest tests[] = {
        {"if (true) { 10 }", true, 10},
        {"if (false) { 10 }", false},
        {"if (1) { 10 }", true, 10},
        {"if (1 < 2) { 10 }", true, 10},
        {"if (1 > 2) { 10 }", false},
        {"if (1 > 2) { 10 } else { 20 }", true, 20},
        {"if (1 < 2) { 10 } else { 20 }", true, 10},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        IfElseTest test = tests[i];
        Object evaluated = test_eval(test.input);
        if (test.has_exp) {
            test_int(evaluated, test.exp);
        } else {
            test_null(evaluated);
        }
    }
}

TEST(Eval, Return) {
    IntTest tests[] = {
        {"return 10;", 10},
        {"return 10; 9;", 10},
        {"return 2 * 5; 9;", 10},
        {"9; return 2 * 5; 9;", 10},
        {"if (10 > 1) {\
            if (10 > 1) {\
                return 10;\
            }\
            return 1;\
        }",
         10},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        IntTest test = tests[i];
        Object evaluated = test_eval(test.input);
        test_int(evaluated, test.exp);
    }
}

TEST(Eval, Errors) {
    ErrorTest tests[] = {
        {"5 + true;", "type mismatch: INTEGER + BOOLEAN"},
        {"5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"},
        {"-true", "unknown operator: -BOOLEAN"},
        {"true + false;", "unknown operator: BOOLEAN + BOOLEAN"},
        {"5; true + false; 5", "unknown operator: BOOLEAN + BOOLEAN"},
        {"if (10 > 1) { true + false; }",
         "unknown operator: BOOLEAN + BOOLEAN"},
        {"if (10 > 1) {\
                if (10 > 1) {\
                    return true + false;\
                }\
                return 1;\
            }",
         "unknown operator: BOOLEAN + BOOLEAN"},
        {"foobar", "identifier not found: foobar"},
    };

    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        ErrorTest test = tests[i];
        Object evaluated = test_eval(test.input);
        EXPECT_EQ(evaluated.type, Object::Type::Error);
        EXPECT_STREQ(std::get<std::string>(evaluated.value).c_str(), test.exp);
    }
}

TEST(Eval, Let) {
    IntTest tests[] = {
        {"let a = 5; a;", 5},
        {"let a = 5 * 5; a;", 25},
        {"let a = 5; let b = a; b;", 5},
        {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        IntTest test = tests[i];
        Object evaluated = test_eval(test.input);
        test_int(evaluated, test.exp);
    }
}

TEST(Eval, Function) {
    std::string input = "fn(x) { x + 2; };";
    Object evaluated = test_eval(input);
    EXPECT_EQ(evaluated.type, Object::Type::Function);
    auto fn = std::get<Function>(evaluated.value);
    EXPECT_EQ(fn.parameters.size(), 1);
    EXPECT_STREQ(fn.parameters[0].string().c_str(), "x");
    EXPECT_STREQ(fn.body.string().c_str(), "(x + 2)");
}

TEST(Eval, FunctionApplication) {
    IntTest tests[] = {
        {"let identity = fn(x) { x; }; identity(5);", 5},
        {"let identity = fn(x) { return x; }; identity(5);", 5},
        {"let double = fn(x) { x * 2; }; double(5);", 10},
        {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
        {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
        {"fn(x) { x; }(5)", 5},
    };
    size_t i, len = arr_size(tests);
    for (i = 0; i < len; ++i) {
        IntTest test = tests[i];
        Object evaluated = test_eval(test.input);
        test_int(evaluated, test.exp);
    }
}

TEST(Eval, Closures) {
    std::string input = "\
    let newAdder = fn(x) {\
        fn(y) { x + y };\
    };\
    let addTwo = newAdder(2);\
    addTwo(2);";
    Object evaluated = test_eval(input);
    test_int(evaluated, 4);
}
