// This project's headers
#include "assembler.h"
#include "code_gen.h"
#include "parser.h"

// Standard headers
#include <stdio.h>
#include <stdint.h>


typedef int(*two_in_one_out)(int, int);


static void run_test(char const *source_code) {
    printf("--- Parsing Code: \"%s\" ---\n", source_code);
    ast_node_t *ast = parser_parse(source_code);
    if (!ast) return;

    printf("--- Abstract Syntax Tree ---\n");
    parser_print_ast_node(ast, 0);

    code_gen(ast);

    two_in_one_out funcPtr = (two_in_one_out)g_assembler.binary;
    int result = funcPtr(1, 2);
    printf("%d\n", result);

    parser_free_ast(ast);
    printf("\n");
}

int main() {
    run_test("{ u64 x; x = 3; u64 y; y = 7; }");
//     run_test(
//         "{"
//         "   u64 i; i = 0;"
//         "   while (i != 10) {"
//         "       i = i + 1;"
//         "   }"
//         "}");
//	  run_test("puts(\"hello\");");

    return 0;
}
