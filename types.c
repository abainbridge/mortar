// Own header
#include "types.h"


hashtab_t g_types;


void types_init(void) {
    g_types = hashtab_create();

    static strview_t sv;

    static type_info_t u8 = { 1 };
    sv = strview_create_from_cstring("u8");
    hashtab_put(&g_types, &sv, &u8);

    static type_info_t u64 = { 8 };
    sv = strview_create_from_cstring("u64");
    hashtab_put(&g_types, &sv, &u64);
}
