#include "util.h"

int bytes_needed_unsigned(unsigned long long n) {
    int bits = 0;

    if (n == 0) return 1; /* zero still needs 1 byte */

    while (n) {
        bits++;
        n >>= 1;
    }

    return (bits + 7) / 8;
}

// helper: convert hex digit to value, assumes valid hex char
int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1; /* invalid */
}

long parse_constant(const char* s, int* isChar) {
    long result = 0;
    int  i;
    *isChar = 0;
    if (s == 0 || *s == '\0') return 0;

    //  Hex constant X'F1'
    if ((s[0] == 'X' || s[0] == 'x') && s[1] == '\'') {
        i = 2;
        while (s[i] && s[i] != '\'') {
            result = result * 16 + hex_val(s[i]);
            i++;
        }
        return result;
    }

    //  Char constant C'EOF
    if ((s[0] == 'C' || s[0] == 'c') && s[1] == '\'') {
        i = 2;
        while (s[i] && s[i] != '\'') i++;
        *isChar = 1;
        return i - 2;  // number of characters
    }

    //  Decimal number
    i = 0;
    while (s[i]) {
        if (!isdigit((unsigned char)s[i])) break;
        result = result * 10 + (s[i] - '0');
        i++;
    }

    return result;
}
