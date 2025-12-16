#pragma once

// This project's headers
#include "common.h"
#include "tokenizer.h"


typedef enum {
    REG_RAX,
    REG_RCX,
    REG_AL,
} asm_reg_t;


typedef struct {
    u8 *binary;
    unsigned binary_size;
} assembler_t;



extern assembler_t g_assembler;

void asm_init(void);

// Function entry/exit
void asm_emit_func_entry(void);
void asm_patch_func_entry(unsigned func_entry_offset, unsigned stack_frame_num_bytes);
void asm_emit_func_exit(void);

// Stack instructions
void asm_emit_stack_alloc(u8 num_bytes);
void asm_emit_stack_dealloc(u8 num_bytes);
void asm_emit_mov_reg_to_stack(asm_reg_t src_reg, unsigned stack_offset);
void asm_emit_mov_stack_to_reg(asm_reg_t dst_reg, unsigned stack_offset);
void asm_emit_zero_stack_range(unsigned stack_offset, unsigned num_bytes);

// Non stack moves
void asm_emit_mov_reg_reg(asm_reg_t dst_reg, asm_reg_t src_reg);
void asm_emit_mov_imm_64(asm_reg_t dst_reg, u64 val);

// Function calls
void asm_emit_call_rax(void);
void asm_emit_ret(void);

// Comparisons
void asm_emit_cmp_imm(asm_reg_t lhs_reg, asm_reg_t rhs_reg);
void asm_patch_cmp_imm(unsigned offset, i64 imm);

// Jumps
void asm_emit_jmp_imm(unsigned target_offset);
void asm_emit_je(unsigned target_offset);
void asm_patch_je(unsigned offset_to_patch, unsigned target_offset);

// Arithmetic/logic
void asm_emit_arithmetic(asm_reg_t dst_reg, asm_reg_t src_reg, TokenType operation);
