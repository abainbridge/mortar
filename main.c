#include "assembler.h"
#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>


typedef int(*two_in_one_out)(int, int);
typedef int(*puts_ptr)(const char* str);


static void foo(char const *s) {
    puts(s);
}

static void code_gen(void) {
    asm_init();

    // Allocate 32-byte stack shadow space
    asm_emit_stack_alloc(32);

    // mov rdx, msg
    asm_emit_mov_imm_64(REG_RCX, (uint64_t)"hello");
    
    // mov rax, &puts
    asm_emit_mov_imm_64(REG_RAX, (uint64_t)puts);

    // call rax
    asm_emit_call_rax();

    // Deallocate the stack shadow space
    asm_emit_stack_dealloc(32);

    // Emit ret
    asm_emit_ret();

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
    parser_free_ast(ast);
    printf("\n");

    code_gen();
}

int main() {
//    run_test("x = 10 + 5; x == 2;");
//    run_test("i32 x; x = 3; foo(x);");

    run_test("puts(\"hello\");");
//    code_gen();

    return 0;
}
