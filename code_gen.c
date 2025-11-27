// Own header
#include "code_gen.h"

// This project's headers
#include "assembler.h"
#include "parser.h"

// Standard headers
#include <assert.h>
#include <stdio.h>


static void code_gen_node(ast_node_t *node);

static void code_gen_block(ast_node_t *node) {
    for (unsigned i = 0; i < node->block.statements.size; i++) {
        code_gen_node(node->block.statements.data[i]);
    }
}

static void code_gen_string_literal(ast_node_t *node) {
    // Put string_addr in rax
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)node->string_literal.data);

    // Write address of literal to stack
    asm_emit_push_reg(REG_RAX);
}

static void code_gen_function_call(ast_node_t *node) {
    //   Alloc stack shadow space
    //   Put address of func to call in rax
    //   Call rax
    //   Dealloc stack shadow space

    assert(node->type == NODE_FUNCTION_CALL);

    // Visit each child node. Each visit will result in an integer/pointer on the stack.
    assert(node->func_call.parameters.size <= 4);
    for (unsigned i = 0; i < node->func_call.parameters.size; i++) {
        code_gen_node(node->func_call.parameters.data[i]);
    }

    // Get parameters from the stack and into registers RCX, RDX, R8, R9.
    asm_reg_t dst_reg = REG_RCX;
    for (unsigned i = 0; i < node->func_call.parameters.size; i++) {
        asm_emit_pop_reg(dst_reg + i);
    }

    // Allocate 32-byte stack shadow space
    asm_emit_stack_alloc(32);

    // mov rax, &puts
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)puts);

    // call rax
    asm_emit_call_rax();

    // Deallocate the stack shadow space
    asm_emit_stack_dealloc(32);

    // Emit ret
    asm_emit_ret();
}

static void code_gen_node(ast_node_t *node) {
    switch (node->type) {
    case NODE_BLOCK:
        code_gen_block(node);
        break;
    case NODE_STRING_LITERAL:
        code_gen_string_literal(node);
        break;
    case NODE_FUNCTION_CALL:
        code_gen_function_call(node);
        break;
    }
}

void code_gen(ast_node_t *ast) {
    asm_init();
    code_gen_node(ast);
}
