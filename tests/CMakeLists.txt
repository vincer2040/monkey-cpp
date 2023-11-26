include(FetchContent)
FetchContent_Declare(
    googletest
    URL     https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    DOWNLOAD_EXTRACT_TIMESTAMP true  # Specify the option here
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
   lexer_test
   lexer_test.cc
)

add_executable(
   parser_test
   parser_test.cc
)

add_executable(
    eval_test
    eval_test.cc
)

target_link_libraries(
    lexer_test
    GTest::gtest_main
    lexer
)

target_link_libraries(
    parser_test
    GTest::gtest_main
    parser
)

target_link_libraries(
    eval_test
    GTest::gtest_main
    parser
    eval
)

include(GoogleTest)
gtest_discover_tests(lexer_test)
gtest_discover_tests(parser_test)
gtest_discover_tests(eval_test)
