// Own header
#include "hash_table.h"

// This project's headers
#include "strview.h"

// Standard headers
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


static unsigned get_idx(hashtab_t const *ht, strview_t const *key) {
    // Hash the key to get the first possible index.
    unsigned idx = 5381;
    for (unsigned i = 0; i < key->len; i++)
        idx = idx * 33 + key->data[i];
    idx &= ht->mask;

    // Linear Probing loop
    while (ht->entries[idx].key.len != 0) {
        if (strview_cmp(&ht->entries[idx].key, key))
            return idx;
        idx = (idx + 1) & ht->mask;
    }

    return idx; // Found an empty slot
}

static void resize_table(hashtab_t* ht) {
    unsigned old_capacity = ht->capacity;
    hashtab_entry_t *old_entries = ht->entries;

    // Calculate new capacity and allocate a new, larger array
    ht->capacity *= 2;
    ht->entries = calloc(ht->capacity, sizeof(hashtab_entry_t));

    // Reset count before re-inserting
    ht->count = 0;

    // Rehash all existing entries into the new, larger table
    for (unsigned i = 0; i < old_capacity; i++) {
        if (old_entries[i].key.len != 0) {
            // Re-insert the old entry into the new table
            unsigned new_index = get_idx(ht, &old_entries[i].key);
            ht->entries[new_index] = old_entries[i];
            ht->count++;
        }
    }

    free(old_entries);
}


// *** Public Functions ***

hashtab_t hashtab_create(void) {
    hashtab_t ht = {0};
    ht.capacity = 16;
    ht.mask = ht.capacity - 1;
    ht.entries = calloc(ht.capacity, sizeof(hashtab_entry_t));
    return ht;
}

void hashtab_put(hashtab_t* ht, strview_t const *key, void *value) {
    if (ht->count >= ht->capacity * 0.7)
        resize_table(ht);

    unsigned idx = get_idx(ht, key);

    // Case 1: Key already exists (Update)
    if (ht->entries[idx].key.len != 0) {
        // Free the old value if needed (application-specific).
        // Here, we just replace the value. The key remains.
        ht->entries[idx].value = value;
    }
    // Case 2: New key (Insert)
    else {
        ht->entries[idx].key = *key;
        ht->entries[idx].value = value;
        ht->count++;
    }
}

void *hashtab_get(hashtab_t const *ht, strview_t const *key) {
    unsigned idx = get_idx(ht, key);
    if (ht->entries[idx].key.len == 0)
        return NULL;
    return ht->entries[idx].value;
}
