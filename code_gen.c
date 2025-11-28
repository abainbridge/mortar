// Own header
#include "code_gen.h"

// This project's headers
#include "assembler.h"
#include "parser.h"
#include "stack_frame.h"
#include "types.h"

// Standard headers
#include <assert.h>
#include <stdio.h>



// Expression evaluation is done in the most brain-dead way possible. Each
// operation reads its inputs from the stack, does the operation and writes
// the result back to the stack.


static void gen_node(ast_node_t *node);

static void gen_assignment(ast_node_t *node) {
    ast_node_t *right = node->assignment.right;
    gen_node(right);

    // Pop result of expression evaluation off the stack, into RAX.
    asm_emit_pop_reg(REG_RAX);

    // Get address of LHS
    ast_node_t *left = node->assignment.left;
    assert(left->type == NODE_IDENTIFIER);
    unsigned offset = sframe_get_variable_offset(&left->identifier.name);

    // Copy RAX to that address on the stack.
    asm_emit_mov_reg_to_stack(REG_RAX, offset);
}

static void gen_block(ast_node_t *node) {
    for (unsigned i = 0; i < node->block.statements.size; i++) {
        gen_node(node->block.statements.data[i]);
    }
}

static void gen_string_literal(ast_node_t *node) {
    // Put string_addr in rax
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)node->string_literal.val.data);

    // Write address of literal to stack
    asm_emit_push_reg(REG_RAX);
}

static void gen_function_call(ast_node_t *node) {
    assert(node->type == NODE_FUNCTION_CALL);

    // Visit each child node. Each visit will result in an integer/pointer on the stack.
    assert(node->func_call.parameters.size <= 4);
    for (unsigned i = 0; i < node->func_call.parameters.size; i++)
        gen_node(node->func_call.parameters.data[i]);

    // Get parameters from the stack and into registers RCX, RDX, R8, R9.
    asm_reg_t dst_reg = REG_RCX;
    for (unsigned i = 0; i < node->func_call.parameters.size; i++)
        asm_emit_pop_reg(dst_reg + i);

    // Allocate 32-byte stack shadow space
    asm_emit_stack_alloc(32);

    // Put address of func to call in rax
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)puts);

    // call rax
    asm_emit_call_rax();

    // Deallocate the stack shadow space
    asm_emit_stack_dealloc(32);

    // Emit ret
    asm_emit_ret();
}

static void gen_variable_declaration(ast_node_t *node) {
    sframe_add_variable(&node->var_decl.identifier_name, node->var_decl.type_info->num_bytes);
}

static void gen_node(ast_node_t *node) {
    switch (node->type) {
    case NODE_NUMBER:
        asm_emit_push_imm_64(node->number.int_value);
        break;
    case NODE_ASSIGNMENT:
        gen_assignment(node);
        break;
    case NODE_BLOCK:
        gen_block(node);
        break;
    case NODE_STRING_LITERAL:
        gen_string_literal(node);
        break;
    case NODE_FUNCTION_CALL:
        gen_function_call(node);
        break;
    case NODE_VARIABLE_DECLARATION:
        gen_variable_declaration(node);
        break;
    default:
        printf("gen_node() unknown type\n");
        assert(0);
    }
}

void code_gen(ast_node_t *ast) {
    asm_init();
    sframe_init();

    asm_emit_func_entry();
    gen_node(ast);
    asm_emit_func_exit(sframe_get_size());
}
