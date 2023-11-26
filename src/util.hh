#pragma once
#include <assert.h>
#include <memory>
#include <string>
#include <stdexcept>

#define unreachable                                                            \
    do {                                                                       \
        assert(0);                                                             \
    } while (0)

bool is_letter(char ch);
bool is_digit(char ch);
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args );
