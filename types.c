// Own header
#include "types.h"


hashtab_t g_types;


void types_init(void) {
    g_types = hashtab_create();
    static type_info_t ti = { 8 };
    static strview_t sv;
    sv = strview_create_from_cstring("u64");
    hashtab_put(&g_types, &sv, &ti);
    type_info_t *ti2 = hashtab_get(&g_types, &sv);
}
