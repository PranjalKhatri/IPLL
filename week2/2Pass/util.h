#pragma once
#include <ctype.h>
// number of bytes needed to representt the number n
int bytes_needed_unsigned(unsigned long long n);

// helper: convert hex digit to value, assumes valid hex char
int hex_val(char c);
// parse constants, like decimal, X'',C''(sets is char and returns number of
// char)
long parse_constant(const char* s, int* isChar);
