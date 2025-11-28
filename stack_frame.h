#pragma once


typedef struct _strview_t strview_t;


void sframe_init(void);
void sframe_add_variable(strview_t *name, unsigned num_bytes);
unsigned sframe_get_variable_offset(strview_t *name);
unsigned sframe_get_size(void);
