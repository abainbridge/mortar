// Own header
#include "lexical_scope.h"

// This project's headers
#include "hash_table.h"


static hashtab_t g_lscope;

#define EMPTY_ENTRY ((void*)1)

void lscope_init(void) {
    g_lscope = hashtab_create();
}

void lscope_add(strview_t *identifier) {
    hashtab_put(&g_lscope, identifier, EMPTY_ENTRY);
}

bool lscope_contains(strview_t *identifier) {
    void *val = hashtab_get(&g_lscope, identifier);
    return val == EMPTY_ENTRY;
}
