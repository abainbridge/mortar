#pragma once

// A dynamic array
//
// Automatically grows as more data is added.

#include <stdlib.h>

typedef struct AstNode AstNode;

typedef struct {
    AstNode **data;       // The array
    unsigned size;        // Number of elements currently stored
    unsigned capacity;    // Max number of elements that can be stored
} darray_t;


void darray_insert(darray_t* arr, AstNode* element);
void darray_free(darray_t* arr);
