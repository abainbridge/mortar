#include "darray.h"

#include <stdlib.h>


void darray_append(darray_t* arr, ast_node_t* element) {
    // Do we need to grow?
    if (arr->size == arr->capacity) {
        if (arr->capacity == 0) {
            arr->capacity = 2;
        }
        else {
            arr->capacity *= 2;
        }
        arr->data = realloc(arr->data, arr->capacity * sizeof(void*));
    }

    arr->data[arr->size] = element;
    arr->size++;
}

void darray_free(darray_t* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = arr->capacity = 0;
}
