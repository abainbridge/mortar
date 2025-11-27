// This module implements something similar to C++'s std::string_view

#pragma once


#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


typedef struct _strview_t {
    char const *data;
    size_t len;
} strview_t;


#ifdef _MSC_VER
#define INLINE static __inline
#else
#define INLINE static inline
#endif


INLINE strview_t strview_create(char const *data, size_t len) {
    return (strview_t){ data, len };
}

INLINE strview_t strview_create_from_cstring(char const *data) {
    return (strview_t){ data, strlen(data) };
}

INLINE strview_t strview_empty(void) {
    return (strview_t){ "", 0 };
}

INLINE bool strview_cmp(strview_t const *a, strview_t const *b) {
    if (a->len != b->len)
        return false;
    return memcmp(a->data, b->data, a->len) == 0;
}

bool strview_cmp_cstr(strview_t const *a, char const *b);
bool strview_to_int(strview_t *sv, int *out_value);
