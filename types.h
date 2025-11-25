#pragma once

#include "hash_table.h"

typedef struct _type_info_t {
    unsigned num_bytes;
} type_info_t;

extern hashtab_t g_types;

void types_init(void);
