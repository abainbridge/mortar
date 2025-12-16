// Own header
#include "strview.h"

// This project's headers
#include "common.h"


bool strview_cmp_cstr(strview_t const *a, char const *b) {
    size_t b_len = strlen(b);
    if (a->len != b_len)
        return false;
    return memcmp(a->data, b, a->len) == 0;
}

bool strview_to_int(strview_t *sv, int *out_value) {
    if (!sv || sv->len == 0)
        return false;

    bool neg = false;
    size_t i = 0;
    i64 value = 0; // use wider type to detect overflow
    char const *s = sv->data;
    size_t len = sv->len;

    // Optional sign
    if (s[0] == '+' || s[0] == '-') {
        neg = (s[0] == '-');
        i++;
        if (i == len) // sign only, no digits
            return false;
    }

    for (; i < len; i++) {
        char c = s[i];
        if (c < '0' || c > '9')
            return false; // invalid character
        value = value * 10 + (c - '0');

        // Check overflow beyond 32-bit signed integer
        if (!neg && value > INT32_MAX)
            return false;
        if (neg && -value < INT32_MIN)
            return false;
    }

    *out_value = neg ? -(int32_t)value : (int32_t)value;
    return true;
}
