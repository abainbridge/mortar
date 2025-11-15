#include "assembler.h"
#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>


typedef int(*two_in_one_out)(int, int);


static void code_gen_node(AstNode *node);

static void code_gen_block(AstNode *node) {
    for (unsigned i = 0; i < node->block.statements.size; i++) {
        code_gen_node(node->block.statements.data[i]);
    }
}

static void code_gen_string_literal(AstNode *node) {
    // Put string_addr in rax
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)node->string_literal.data);

    // Write address of literal to stack
    asm_emit_push_reg(REG_RAX);
}

static void code_gen_function_call(AstNode *node) {
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

static void code_gen_node(AstNode *node) {
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

static void code_gen(AstNode *ast) {
    asm_init();

    code_gen_node(ast);

    two_in_one_out funcPtr = (two_in_one_out)g_assembler.binary;
    int result = funcPtr(1, 2);
    printf("%d\n", result);
}

static void run_test(char const *source_code) {
    printf("--- Parsing Code: \"%s\" ---\n", source_code);
    AstNode *ast = parser_parse(source_code);
    if (!ast) return;

    printf("--- Abstract Syntax Tree ---\n");
    parser_print_ast_node(ast, 0);

    code_gen(ast);

    parser_free_ast(ast);
    printf("\n");
}

int main() {
//    run_test("x = 10 + 5; x == 2;");
//    run_test("i32 x; x = 3; foo(x);");

    run_test("puts(\"hello\");");
//    code_gen();

    return 0;
}
