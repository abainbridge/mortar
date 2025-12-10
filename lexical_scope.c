// Own header
#include "lexical_scope.h"

// This project's headers
#include "hash_table.h"
#include "types.h"


static hashtab_t g_lscope;


void lscope_init(void) {
    g_lscope = hashtab_create();
}

void lscope_add(strview_t *identifier, derived_type_t *type) {
    derived_type_t *copied = malloc(sizeof(derived_type_t));
    *copied = *type;
    hashtab_put(&g_lscope, identifier, copied);
}

derived_type_t *lscope_get(strview_t *identifier) {
    return hashtab_get(&g_lscope, identifier);
}
