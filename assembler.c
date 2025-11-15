// Own header
#include "assembler.h"

// Standard headers
#include <string.h>


void *VirtualAlloc(void *address, size_t size, unsigned allocationType, unsigned protect);

enum {
    MEM_COMMIT = 0x1000,
    MEM_RESERVE = 0x2000,
    PAGE_EXECUTE_READWRITE = 0x40
};


assembler_t g_assembler;


void asm_init(void) {
    g_assembler.binary = VirtualAlloc(NULL, 0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);
}

void asm_emit_stack_alloc(uint8_t num_bytes) {
    // Emit sub rsp, 0x20
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
    case REG_RDX: c[0] = 0x51; break;
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

void asm_emit_mov_imm_64(asm_reg_t dst_reg, uint64_t val) {
    uint8_t *c = g_assembler.binary + g_assembler.binary_size;
    c[0] = 0x48;
    
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
