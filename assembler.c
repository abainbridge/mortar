// Own header
#include "assembler.h"

// This project's headers
#include "common.h"

// Standard headers
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


void *VirtualAlloc(void *address, size_t size, unsigned allocationType, unsigned protect);

enum {
    MEM_COMMIT = 0x1000,
    MEM_RESERVE = 0x2000,
    PAGE_EXECUTE_READWRITE = 0x40
};


assembler_t g_assembler;


static bool fits_in_s8(int64_t val) {
    return (val <= INT8_MAX && val >= INT8_MIN);
}

static bool fits_in_s32(int64_t val) {
    return (val < INT32_MAX || val >= INT32_MIN);
}

void asm_init(void) {
    g_assembler.binary = VirtualAlloc(NULL, 0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);
}

static void emit_bytes(void *bytes, unsigned num_bytes) {
    uint8_t *o = g_assembler.binary + g_assembler.binary_size;
    memcpy(o, bytes, num_bytes);
    g_assembler.binary_size += num_bytes;
}

void asm_emit_func_entry(void) {
    assert(g_assembler.sub_rsp_idx == 0);

    // Function entry is always:
    //  push   rbp
    //  mov    rbp,rsp
    //  sub    rsp,??? # Will be overwritten once we know the size of the stack frame

    uint8_t c[] = {
        0x55, // push rbp

        0x48, // move rbp,rsp
        0x89,
        0xe5,

        0x48, // sub rsp,????
        0x81,
        0xec

        // The next 4 bytes store the amount to subtract from rsp. We will leave
        // uninitialized. We will patch them with the correct value once the stack
        // frame size is known.
    };

    emit_bytes(c, 11);
}

void asm_patch_func_entry(unsigned start_of_code, unsigned stack_frame_num_bytes) {
    uint32_t *c = (uint32_t *)(g_assembler.binary + start_of_code + 7);
    *c = stack_frame_num_bytes;
}

void asm_emit_func_exit(void) {
    // Function exit is always:
    //  leave
    //  ret
    uint8_t c[] = { 0xc9, 0xc3 };
    emit_bytes(c, 2);
}

void asm_emit_stack_alloc(uint8_t num_bytes) {
    // Emit sub rsp, num_bytes
    uint8_t c[] = { 0x48, 0x83, 0xec, num_bytes };
    emit_bytes(c, 4);
}

void asm_emit_stack_dealloc(uint8_t num_bytes) {
    // Emit add rsp, 0x20
    uint8_t c[] = { 0x48, 0x83, 0xc4, num_bytes };
    emit_bytes(c, 4);
}

void asm_emit_mov_reg_to_stack(asm_reg_t src_reg, unsigned stack_offset) {
    int64_t neg_stack_offset = -((int64_t)stack_offset);
    if (!fits_in_s8(neg_stack_offset))
        DBG_BREAK();

    uint8_t c[4];
    switch (src_reg) {
    case REG_RAX:
    case REG_RCX:
        c[0] = 0x48; // Prefix for 64-bit operation
        c[1] = 0x89; // Move reg to memory/register
        c[2] = 0x45 + (src_reg << 3);
        c[3] = (uint8_t)neg_stack_offset;
        emit_bytes(c, 4);
        break;
    case REG_AL:
        c[0] = 0x88;
        c[1] = 0x45;
        c[2] = (uint8_t)neg_stack_offset;
        emit_bytes(c, 3);
    }
}

void asm_emit_mov_stack_to_reg(asm_reg_t dst_reg, unsigned stack_offset) {
    uint8_t c[4];
    int64_t neg_stack_offset = -((int64_t)stack_offset);
    if (!fits_in_s8(neg_stack_offset))
        DBG_BREAK();

    if (dst_reg == REG_AL) {
        // Copy 8-bit value from stack to 64-bit register (Dest is RAX not AL)
        c[0] = 0xf;
        c[1] = 0xb6;
        c[2] = 0x45;
    }
    else {
        // Copy 64-bit value from stack to 64-bit register
        c[0] = 0x48; // Prefix for 64-bit operation
        c[1] = 0x8b; // Move memory/register to register
        c[2] = 0x45; // Mod=01, Reg=000 (RAX), R/M=101 (RBP)
    }

    c[3] = (uint8_t)neg_stack_offset;

    emit_bytes(c, 4);
}

void asm_emit_mov_reg_reg(asm_reg_t dst_reg, asm_reg_t src_reg) {
    uint8_t c[3] = { 0x48, 0x89 };
    c[2] = 0xc0 | (src_reg << 3) | dst_reg; // 0xc0 = ModR/M byte: register-direct mode
    emit_bytes(c, 3);
}

void asm_emit_mov_imm_64(asm_reg_t dst_reg, uint64_t val) {
    uint8_t c[2] = { 0x48 };
    
    switch (dst_reg) {
    case REG_RAX: c[1] = 0xb8; break;
    case REG_RCX: c[1] = 0xb9; break;
    }

    emit_bytes(c, 2);
    emit_bytes(&val, 8);
}

void asm_emit_call_rax(void) {
    uint8_t c[] = { 0xff, 0xd0 };
    emit_bytes(c, 2);
}

void asm_emit_ret(void) {
    uint8_t c[] = { 0xc3 };
    emit_bytes(c, 1);
}

void asm_emit_cmp_imm(asm_reg_t lhs_reg, asm_reg_t rhs_reg) {
    uint8_t c[3] = { 0x48, 0x39, 0xc0 };
    c[2] |= (lhs_reg << 3) | rhs_reg;
    emit_bytes(c, 3);
}

void asm_emit_jmp_imm(unsigned target_offset) {
    int64_t rel_offset = (int64_t)target_offset - (int64_t)g_assembler.binary_size - 5;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();

    int32_t rel_offset32 = (int32_t)rel_offset;
    uint8_t c[] = { 0xe9 };
    emit_bytes(c, 1);
    emit_bytes(&rel_offset32, 4);
}

void asm_emit_je(unsigned target_offset) {
    int64_t rel_offset = (int64_t)target_offset - (int64_t)g_assembler.binary_size - 6;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();
        
    int32_t rel_offset32 = (int32_t)rel_offset;
    uint8_t c[] = { 0x0f, 0x84 };
    emit_bytes(c, 2);
    emit_bytes(&rel_offset32, 4);
}

void asm_patch_je(unsigned offset_to_patch, unsigned target_offset) {
    int64_t rel_offset = (int64_t)target_offset - (int64_t)offset_to_patch - 6;
    uint8_t *c = g_assembler.binary + offset_to_patch;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();

    int32_t rel_offset32 = (int32_t)rel_offset;
    memcpy(&c[2], &rel_offset32, 4);
}

void asm_emit_arithmetic(asm_reg_t dst_reg, asm_reg_t src_reg, TokenType operation) {
    uint8_t c[3] = { 0x48 };
    
    switch (operation) {
    case TOKEN_PLUS:
        c[1] = 0x01; // ADD r/m64, r64
        break;
    default:
        printf("Unknown arithmetic operation\n");
        DBG_BREAK();
    }
    
    c[2] = 0xc0 | // Mod = 11
        (REG_RCX << 3) |
        REG_RAX;
    emit_bytes(c, 3);
}
