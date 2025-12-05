#pragma once

#include <stdbool.h>

typedef struct _strview_t strview_t;

void lscope_init(void);
void lscope_add(strview_t *identifier);
bool lscope_contains(strview_t *identifier);
