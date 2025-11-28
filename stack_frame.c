// Own header
#include "stack_frame.h"

// This project's headers
#include "common.h"
#include "hash_table.h"
#include "strview.h"


enum { MAX_SFRAME_ITEMS = 100 };


typedef struct {
    strview_t *name;
    unsigned offset;
} sframe_item_t;

typedef struct {
    sframe_item_t items[MAX_SFRAME_ITEMS];
    unsigned num_items;
    unsigned current_offset;
} sframe_t;


static sframe_t g_sframe;


void sframe_init(void) {
    g_sframe.num_items = 0;
    g_sframe.current_offset = 0;
}

void sframe_add_variable(strview_t *name, unsigned num_bytes) {
    if (g_sframe.num_items >= MAX_SFRAME_ITEMS)
        FATAL_ERROR("Too many items in current stack frame. Limit is %d\n", MAX_SFRAME_ITEMS);

    g_sframe.items[g_sframe.num_items].name = name;
    g_sframe.items[g_sframe.num_items].offset = g_sframe.current_offset;
    g_sframe.num_items++;
    g_sframe.current_offset += num_bytes;
}

unsigned sframe_get_variable_offset(strview_t *name) {
    for (unsigned i = 0; i < g_sframe.num_items; i++) {
        if (strview_cmp(g_sframe.items[i].name, name)) {
            return g_sframe.items[i].offset;
        }
    }

    FATAL_ERROR("Couldn't find storage offset for variable '%.*s'", name->len, name->data);
    return 0;
}

unsigned sframe_get_size(void) {
    return g_sframe.current_offset;
}
