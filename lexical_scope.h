#pragma once

typedef struct _strview_t strview_t;
typedef struct _derived_type_t derived_type_t;

void lscope_init(void);

// Mallocs and copies type. Will be freed when lexical scope is destroyed (todo).
void lscope_add(strview_t *identifier, derived_type_t *type);

derived_type_t *lscope_get(strview_t *identifier);
