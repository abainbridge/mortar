// Own header
#include "assembler.h"

// This project's headers
#include "common.h"

// Standard headers
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
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

    g_assembler.sub_rsp_idx = 0;
}

void asm_emit_func_entry(void) {
    assert(g_assembler.sub_rsp_idx == 0);

    // Function entry is always:
    //  push   rbp
    //  mov    rbp,rsp
    //  sub    rsp,0x10 # Will be overwritten once we know the size of the stack frame
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x55; // push rbp

    c[1] = 0x48; // move rbp,rsp
    c[2] = 0x89;
    c[3] = 0xe5;

    c[4] = 0x48; // sub rsp,????
    c[5] = 0x81;
    c[6] = 0xec;

    // c[7] to c[10] store the amount to subtract from rsp. Leave
    // uninitialized but store their location so that
    // we can patch them with the correct value once the stack frame size
    // is known.
    g_assembler.sub_rsp_idx = g_assembler.binary_size + 7;

    g_assembler.binary_size += 11;
}

void asm_emit_func_exit(unsigned stack_frame_size) {
    // First patch up the function entry using the stack frame size.
    {
        assert(g_assembler.sub_rsp_idx != 0);
        uint32_t *c = (uint32_t *)(g_assembler.binary + g_assembler.sub_rsp_idx);
        *c = stack_frame_size;
        g_assembler.sub_rsp_idx = 0;
    }

    // Function exit is always:
    //  leave
    //  ret
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0xc9;
    c[1] = 0xc3;
    g_assembler.binary_size += 2;
}

void asm_emit_stack_alloc(uint8_t num_bytes) {
    // Emit sub rsp, num_bytes
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48;
    c[1] = 0x83;
    c[2] = 0xEC;
    c[3] = num_bytes;
    g_assembler.binary_size += 4;
}

void asm_emit_stack_dealloc(uint8_t num_bytes) {
    // Emit add rsp, 0x20
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48;
    c[1] = 0x83;
    c[2] = 0xC4;
    c[3] = num_bytes;
    g_assembler.binary_size += 4;
}

void asm_emit_push_reg(asm_reg_t src_reg) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    switch (src_reg) {
    case REG_RAX: c[0] = 0x50; break;
    case REG_RCX: c[0] = 0x51; break;
    case REG_RDX: c[0] = 0x52; break;
    }
    g_assembler.binary_size += 1;
}

void asm_emit_pop_reg(asm_reg_t dst_reg) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    switch (dst_reg) {
    case REG_RAX: c[0] = 0x58; break;
    case REG_RCX: c[0] = 0x59; break;
    case REG_RDX: c[0] = 0x5a; break;
    }
    g_assembler.binary_size += 1;
}

void asm_emit_push_imm_64(int64_t val) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;

    if (fits_in_s8(val)) {
        // push imm8
        c[0] = 0x6a;
        c[1] = (uint8_t)val;
        g_assembler.binary_size += 2;
    }
    else if (fits_in_s32(val)) {
        // pushq
        c[0] = 0x68;
        uint32_t *d = (uint32_t *)(c + 1);
        *d = (uint32_t)val;
        g_assembler.binary_size += 5;
    }
    else {
        // Need to use mov + push
        // 
        // 48 b8 XX XX XX XX XX XX XX XX    # mov rax, 0xXXXXXXXXXXXXXXXX
        // 50                               # push rax
        c[0] = 0x48;
        c[1] = 0xb8;
        uint64_t *d = (uint64_t *)(c + 2);
        *d = val;
        c[10] = 0x50; // push rax
        g_assembler.binary_size += 11;
    }
}

void asm_emit_mov_reg_to_stack(asm_reg_t src_reg, unsigned stack_offset) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48; // Prefix for 64-bit operation
    c[1] = 0x89; // Move reg to memory/register

    c[2] = 0x45;
    // Bits 7-6: Dst is address from register with 8-bit displacement
    // Bits 5-3: Src reg (init to zero)
    // Bits 2-0: Dst address is from register RBP

    switch (src_reg) {
    case REG_RAX: break;
    case REG_RCX: c[2] |= 1 << 3; break;
    case REG_RDX: c[2] |= 2 << 3; break;
    }

    int64_t neg_stack_offset = -((int64_t)stack_offset);
    if (!fits_in_s8(neg_stack_offset))
        DBG_BREAK();
    c[3] = (uint8_t)neg_stack_offset;

    g_assembler.binary_size += 4;
}

void asm_emit_mov_stack_to_reg(asm_reg_t dst_reg, unsigned stack_offset) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48; // Prefix for 64-bit operation
    c[1] = 0x8b; // Move memory/register to register
    c[2] = 0x45; // Mod=01, Reg=000 (RAX), R/M=101 (RBP)

    int64_t neg_stack_offset = -((int64_t)stack_offset);
    if (!fits_in_s8(neg_stack_offset))
        DBG_BREAK();
    c[3] = (uint8_t)neg_stack_offset;

    g_assembler.binary_size += 4;
}

void asm_emit_mov_imm_64(asm_reg_t dst_reg, uint64_t val) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48; // Prefix for 64-bit operation
    
    switch (dst_reg) {
    case REG_RAX: c[1] = 0xb8; break;
    case REG_RCX: c[1] = 0xb9; break;
    case REG_RDX: c[1] = 0xba; break;
    }

    memcpy(c + 2, &val, 8);
    g_assembler.binary_size += 10;
}

void asm_emit_call_rax(void) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0xff;
    c[1] = 0xd0;
    g_assembler.binary_size += 2;
}

void asm_emit_ret(void) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0xc3;
    g_assembler.binary_size += 1;
}

void asm_emit_cmp_imm(asm_reg_t lhs_reg, asm_reg_t rhs_reg) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48;    // 64-bit op
    c[1] = 0x39;    // cmp
    c[2] = 0xc0 |   // register-register
        (lhs_reg << 3) |
        rhs_reg;
    g_assembler.binary_size += 3;
}

void asm_emit_jmp_imm(unsigned target_offset) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;

    int64_t rel_offset = (int64_t)target_offset - (int64_t)g_assembler.binary_size - 5;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();

    c[0] = 0xe9;
    memcpy(&c[1], &rel_offset, 4);
    g_assembler.binary_size += 5;
}

void asm_emit_je(unsigned target_offset) {
    int64_t rel_offset = (int64_t)target_offset - (int64_t)g_assembler.binary_size - 6;
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    if (fits_in_s32(rel_offset)) {
        c[0] = 0x0f; // Two byte opcode prefix
        c[1] = 0x84; // JE
        memcpy(&c[2], &rel_offset, 4);
        g_assembler.binary_size += 6;
    }
    else {
        DBG_BREAK();
    }
}

void asm_patch_je(unsigned offset_to_patch, unsigned target_offset) {
    int64_t rel_offset = (int64_t)target_offset - (int64_t)offset_to_patch - 6;
    uint8_t *c = g_assembler.binary + offset_to_patch;
    if (fits_in_s32(rel_offset)) {
        memcpy(&c[2], &rel_offset, 4);
    }
    else {
        DBG_BREAK();
    }
}

void asm_emit_arithmetic(asm_reg_t dst_reg, asm_reg_t src_reg, TokenType operation) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48; // Prefix for 64-bit operation
    
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
    g_assembler.binary_size += 3;
}
