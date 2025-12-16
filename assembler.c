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


static bool fits_in_s8(i64 val) {
    return (val <= INT8_MAX && val >= INT8_MIN);
}

static bool fits_in_s32(i64 val) {
    return (val < INT32_MAX || val >= INT32_MIN);
}

void asm_init(void) {
    g_assembler.binary = VirtualAlloc(NULL, 0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);
}

static void emit_bytes(void *bytes, unsigned num_bytes) {
    u8 *o = g_assembler.binary + g_assembler.binary_size;
    memcpy(o, bytes, num_bytes);
    g_assembler.binary_size += num_bytes;
}

void asm_emit_func_entry(void) {
    u8 c[] = {
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
    u32 *c = (u32 *)(g_assembler.binary + start_of_code + 7);
    *c = stack_frame_num_bytes;
}

void asm_emit_func_exit(void) {
    u8 c[] = { 0xc9, 0xc3 }; // emit leave; ret
    emit_bytes(c, 2);
}

void asm_emit_stack_alloc(u8 num_bytes) {
    // Emit sub rsp, num_bytes
    u8 c[] = { 0x48, 0x83, 0xec, num_bytes };
    emit_bytes(c, 4);
}

void asm_emit_stack_dealloc(u8 num_bytes) {
    // Emit add rsp, 0x20
    u8 c[] = { 0x48, 0x83, 0xc4, num_bytes };
    emit_bytes(c, 4);
}

void asm_emit_mov_reg_to_stack(asm_reg_t src_reg, unsigned stack_offset) {
    i64 num_bytes = (src_reg == REG_AL ? 1 : 8);
    i64 relative_addr = -(i64)stack_offset - num_bytes;
    if (!fits_in_s8(relative_addr))
        DBG_BREAK();

    switch (src_reg) {
    case REG_RAX:
    case REG_RCX:
        // mov qword ptr [rbp + stack_offset], src_reg
        emit_bytes((u8[]){ 0x48, 0x89, 0x45 + (src_reg << 3), (i8)relative_addr }, 4);
        break;
    case REG_AL:
        // mov byte ptr [rbp + stack_offset], al
        emit_bytes((u8[]){ 0x88, 0x45, (i8)relative_addr}, 3);
    }
}

void asm_emit_mov_stack_to_reg(asm_reg_t dst_reg, unsigned stack_offset) {
    i64 num_bytes = (dst_reg == REG_AL ? 1 : 8);
    i64 relative_addr = -(i64)stack_offset - num_bytes;
    if (!fits_in_s8(relative_addr))
        DBG_BREAK();

    switch (dst_reg) {
    case REG_RAX:
        // mov rax, qword ptr [rbp + relative_addr]
        emit_bytes((u8[]){ 0x48, 0x8b, 0x45, (i8)relative_addr }, 4);
        break;
    case REG_AL:
        // mov al, byte ptr [rbp + relative_addr]
        emit_bytes((u8[]){ 0xf, 0xb6, 0x45, (i8)relative_addr }, 4);
        break;
    }
}

void asm_emit_zero_stack_range(unsigned stack_offset, unsigned num_bytes) {
    i64 relative_addr = -(i64)stack_offset - (i64)num_bytes;
    if (!fits_in_s8(relative_addr))
        DBG_BREAK();

    switch (num_bytes) {
    case 1:
        // xor cl, cl
        emit_bytes((u8[]){ 0x30, 0xc9 }, 2);
        // mov byte ptr [rbp - stack_offset], cl
        emit_bytes((u8[]){ 0x88, 0x4d, (i8)relative_addr }, 3);
        break;
    case 8:
        // xor rcx, rcx
        emit_bytes((u8[]){ 0x48, 0x31, 0xc9 }, 3);
        // mov qword ptr [rbp - stack_offset], rcx
        emit_bytes((u8[]){ 0x48, 0x89, 0x45, (i8)relative_addr }, 4);
        break;
    case 16:
        // pxor xmm0, xmm0
        emit_bytes((u8[]){ 0x66, 0x0f, 0xef, 0xc0 }, 4);
        // movdqu [rbp - stack_offset], xmm0
        emit_bytes((u8[]){ 0xf3, 0x0f, 0x7f, 0x45, (i8)relative_addr }, 5);
        break;
    default:
        DBG_BREAK();
    }
}

void asm_emit_mov_reg_reg(asm_reg_t dst_reg, asm_reg_t src_reg) {
    u8 c[3] = { 0x48, 0x89 };
    c[2] = 0xc0 | (src_reg << 3) | dst_reg; // 0xc0 = ModR/M byte: register-direct mode
    emit_bytes(c, 3);
}

void asm_emit_mov_imm_64(asm_reg_t dst_reg, u64 val) {
    u8 c[2] = { 0x48 };
    
    // todo: optimized versions for immediate that fits in 8 bits, etc.

    switch (dst_reg) {
    case REG_RAX: c[1] = 0xb8; break;
    case REG_RCX: c[1] = 0xb9; break;
    }

    emit_bytes(c, 2);
    emit_bytes(&val, 8);
}

void asm_emit_call_rax(void) {
    u8 c[] = { 0xff, 0xd0 };
    emit_bytes(c, 2);
}

void asm_emit_ret(void) {
    u8 c[] = { 0xc3 };
    emit_bytes(c, 1);
}

void asm_emit_cmp_imm(asm_reg_t lhs_reg, asm_reg_t rhs_reg) {
    u8 c[3] = { 0x48, 0x39, 0xc0 };
    c[2] |= (lhs_reg << 3) | rhs_reg;
    emit_bytes(c, 3);
}

void asm_emit_jmp_imm(unsigned target_offset) {
    int32_t rel_offset32; // VS2013 needs this to be here.
    i64 rel_offset = (i64)target_offset - (i64)g_assembler.binary_size - 5;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();

    rel_offset32 = (int32_t)rel_offset;
    u8 c[] = { 0xe9 };
    emit_bytes(c, 1);
    emit_bytes(&rel_offset32, 4);
}

void asm_emit_je(unsigned target_offset) {
    int32_t rel_offset32; // VS2013 needs this to be here.
    i64 rel_offset = (i64)target_offset - (i64)g_assembler.binary_size - 6;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();
        
    rel_offset32 = (int32_t)rel_offset;
    u8 c[] = { 0x0f, 0x84 };
    emit_bytes(c, 2);
    emit_bytes(&rel_offset32, 4);
}

void asm_patch_je(unsigned offset_to_patch, unsigned target_offset) {
    int32_t rel_offset32; // VS2013 needs this to be here.
    i64 rel_offset = (i64)target_offset - (i64)offset_to_patch - 6;
    u8 *c = g_assembler.binary + offset_to_patch;
    if (!fits_in_s32(rel_offset))
        DBG_BREAK();

    rel_offset32 = (int32_t)rel_offset;
    memcpy(&c[2], &rel_offset32, 4);
}

void asm_emit_arithmetic(asm_reg_t dst_reg, asm_reg_t src_reg, TokenType operation) {
    u8 c[3] = { 0x48 };
    
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
