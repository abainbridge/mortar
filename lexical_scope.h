#pragma once

#include <stdbool.h>

typedef struct _strview_t strview_t;
typedef struct _type_info_t type_info_t;

void lscope_init(void);
void lscope_add(strview_t *identifier, type_info_t *type);
type_info_t *lscope_get(strview_t *identifier);
