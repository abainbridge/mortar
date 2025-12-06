// Own header
#include "lexical_scope.h"

// This project's headers
#include "hash_table.h"


static hashtab_t g_lscope;


void lscope_init(void) {
    g_lscope = hashtab_create();
}

void lscope_add(strview_t *identifier, type_info_t *type) {
    hashtab_put(&g_lscope, identifier, type);
}

type_info_t *lscope_get(strview_t *identifier) {
    return hashtab_get(&g_lscope, identifier);
}
