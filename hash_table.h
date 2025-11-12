#pragma once

#include "strview.h"


typedef struct {
    strview_t key;
    void *value;
} hashtab_entry_t;

typedef struct hashtab_t {
    hashtab_entry_t *entries; // Array of entries (the actual hash table)
    unsigned capacity;        // Size of the entries array. Is always a power of 2.
    unsigned mask;            // capacity - 1
    unsigned count;           // Number of occupied slots
} hashtab_t;


hashtab_t hashtab_create(void);
void hashtab_put(hashtab_t *ht, strview_t const *key, void *val);
void *hashtab_get(hashtab_t const *ht, strview_t const *key);
