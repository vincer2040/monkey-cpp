#pragma once
#include <assert.h>

#define unreachable                                                            \
    do {                                                                       \
        assert(0);                                                             \
    } while (0)

bool is_letter(char ch);
bool is_digit(char ch);
