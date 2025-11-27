#pragma once

// A dynamic array
//
// Automatically grows as more data is added.

#include <stdlib.h>

typedef struct _ast_node_t ast_node_t;

typedef struct {
    ast_node_t **data;       // The array. NULL if capacity is 0.
    unsigned size;        // Number of elements currently stored
    unsigned capacity;    // Max number of elements that can be stored
} darray_t;


void darray_insert(darray_t* arr, ast_node_t* element);
void darray_free(darray_t* arr);
