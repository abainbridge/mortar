// Own header
#include "types.h"


static hashtab_t g_types;


void types_init(void) {
    g_types = hashtab_create();

    static strview_t sv;

    static object_type_t u8 = { 1 };
    sv = strview_create_from_cstring("u8");
    hashtab_put(&g_types, &sv, &u8);

    static object_type_t u64 = { 8 };
    sv = strview_create_from_cstring("u64");
    hashtab_put(&g_types, &sv, &u64);
}

object_type_t *types_get_obj_type(strview_t *type_name) {
    return hashtab_get(&g_types, type_name);
}
