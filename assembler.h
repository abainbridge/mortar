#pragma once


#include <stdint.h>


typedef enum {
    REG_RAX,
    REG_RCX,
    REG_RDX
} asm_reg_t;


typedef struct {
    uint8_t *binary;
    unsigned binary_size;
} assembler_t;



extern assembler_t g_assembler;

void asm_init(void);
void asm_emit_stack_alloc(uint8_t num_bytes);
void asm_emit_stack_dealloc(uint8_t num_bytes);
void asm_emit_push_reg(asm_reg_t src_reg);
void asm_emit_pop_reg(asm_reg_t dst_reg);
void asm_emit_mov_imm_64(asm_reg_t dst_reg, uint64_t val);
void asm_emit_call_rax(void);
void asm_emit_ret(void);
