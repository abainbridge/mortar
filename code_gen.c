// Own header
#include "code_gen.h"

// This project's headers
#include "assembler.h"
#include "common.h"
#include "lexical_scope.h"
#include "parser.h"
#include "stack_frame.h"
#include "types.h"

// Standard headers
#include <assert.h>
#include <stdio.h>



// Expression evaluation is done in the most brain-dead way possible. Each
// operation reads an input from rax and rcx and writes its result to rax.
// Everything is promoted to u64 before use in the expression evaluation.


static void gen_node(ast_node_t *node);

static void gen_assignment(ast_node_t *node) {
    ast_node_t *right = node->assignment.right;
    gen_node(right);

    // Get address of LHS
    ast_node_t *left = node->assignment.left;
    assert(left->type == NODE_IDENTIFIER);
    unsigned offset = sframe_get_variable_offset(&left->identifier.name);

    // Get type of LHS
    derived_type_t *type = lscope_get(&left->identifier.name);
    assert(type);

    // Copy RAX or AL to that address on the stack.
    if (type->object_type.num_bytes == 1) {
        asm_emit_mov_reg_to_stack(REG_AL, offset);
    }
    else {
        asm_emit_mov_reg_to_stack(REG_RAX, offset);
    }
}

static void gen_binary_op(ast_node_t *node) {
    gen_node(node->binary_op.left);
    asm_emit_mov_reg_reg(REG_RCX, REG_RAX);
    gen_node(node->binary_op.right);

    switch (node->binary_op.op) {
    case TOKEN_PLUS:
        asm_emit_arithmetic(REG_RAX, REG_RCX, node->binary_op.op);
        break;
    default:
        printf("Unknown binary op\n");
        DBG_BREAK();
    }
}

// This function is only used to read from an identifier.
static void gen_identifier(ast_node_t *node) {
    derived_type_t *type = lscope_get(&node->identifier.name);
    assert(type);
    unsigned offset = sframe_get_variable_offset(&node->identifier.name);
    if (type->object_type.num_bytes == 1)
        asm_emit_mov_stack_to_reg(REG_AL, offset);
    else
        asm_emit_mov_stack_to_reg(REG_RAX, offset);
}

static void gen_compare(ast_node_t *node) {
    gen_node(node->compare_op.left);
    asm_emit_mov_reg_reg(REG_RCX, REG_RAX);
    gen_node(node->compare_op.right);
    asm_emit_cmp_imm(REG_RAX, REG_RCX);
}

static void gen_block(ast_node_t *node) {
    for (unsigned i = 0; i < node->block.statements.size; i++) {
        gen_node(node->block.statements.data[i]);
    }
}

static void gen_string_literal(ast_node_t *node) {
    // Put string_addr in rax
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)"Hello");// node->string_literal.val.data);
}

static void gen_function_call(ast_node_t *node) {
    assert(node->type == NODE_FUNCTION_CALL);

    // Visit each child node. Each visit will result in an integer/pointer on the stack.
    assert(node->func_call.parameters.size <= 4);
    asm_reg_t dst_reg = REG_RCX;
    for (unsigned i = 0; i < node->func_call.parameters.size; i++) {
        gen_node(node->func_call.parameters.data[i]);
        // todo
        // Get parameters from the stack and into registers RCX, RDX, R8, R9.
//         asm_emit_mov_reg_reg(dst_reg, REG_RAX);
        dst_reg++;
    }

    // Allocate 32-byte stack shadow space
    asm_emit_stack_alloc(32);

    // Put address of func to call in rax
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)puts);

    // call rax
    asm_emit_call_rax();

    // Deallocate the stack shadow space
    asm_emit_stack_dealloc(32);
}

static void gen_variable_declaration(ast_node_t *node) {
    if (node->var_decl.type_info.is_array) {
    }
    else {
        sframe_add_variable(&node->var_decl.identifier_name, node->var_decl.type_info.object_type.num_bytes);
    }
}

static void gen_while_loop(ast_node_t *node) {
    unsigned start_of_condition = g_assembler.binary_size;
    
    gen_node(node->while_loop.condition_expr);

    unsigned jeq_end_offset = g_assembler.binary_size;
    asm_emit_je(0);

    gen_block(node->while_loop.block);

    asm_emit_jmp_imm(start_of_condition);

    asm_patch_je(jeq_end_offset, g_assembler.binary_size);
}

static void gen_node(ast_node_t *node) {
    switch (node->type) {
    case NODE_NUMBER:
        asm_emit_mov_imm_64(REG_RAX, node->number.int_value);
        break;
    case NODE_IDENTIFIER:
        gen_identifier(node);
        break;
    case NODE_ASSIGNMENT:
        gen_assignment(node);
        break;
    case NODE_BINARY_OP:
        gen_binary_op(node);
        break;
    case NODE_COMPARE:
        gen_compare(node);
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
    case NODE_WHILE:
        gen_while_loop(node);
        break;
    default:
        printf("gen_node() unknown type\n");
        DBG_BREAK();
    }
}

void code_gen(ast_node_t *ast) {
    asm_init();
    sframe_init();

    unsigned start_of_code = g_assembler.binary_size;
    asm_emit_func_entry();
    gen_node(ast);
    asm_patch_func_entry(start_of_code, sframe_get_size());
    asm_emit_func_exit();
}
