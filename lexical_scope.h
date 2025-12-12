#pragma once

typedef struct _strview_t strview_t;
typedef struct _derived_type_t derived_type_t;

void lscope_init(void);

// The AST owns the data passed in. This module only stores the pointers.
void lscope_add(strview_t *identifier, derived_type_t *type);

derived_type_t *lscope_get(strview_t *identifier);
