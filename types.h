#pragma once

#include "hash_table.h"

// A type that is NOT derived from another type. eg u8, or (todo) a user defined struct.
typedef struct _type_info_t {
    unsigned num_bytes;
} object_type_t;

typedef struct _derived_type_t {
    object_type_t object_type;
    bool is_array;
} derived_type_t;

void types_init(void);
object_type_t *types_get_obj_type(strview_t *type_name);
