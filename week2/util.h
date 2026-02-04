#pragma once
#include <ctype.h>
int  bytes_needed_unsigned(unsigned long long n);

// helper: convert hex digit to value, assumes valid hex char
int  hex_val(char c);
long parse_constant(const char* s, int* isChar);
